#include "undoable_model_state_pair.h"

#include <libopensimcreator/documents/model/environment.h>
#include <libopensimcreator/documents/model/model_state_commit.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/platform/log.h>
#include <liboscar/utilities/algorithms.h>
#include <liboscar/utilities/assertions.h>
#include <liboscar/utilities/perf.h>
#include <liboscar/utilities/uid.h>
#include <OpenSim/Common/ComponentPath.h>
#include <OpenSim/Common/ModelDisplayHints.h>
#include <OpenSim/Common/PropertyObjArray.h>
#include <OpenSim/Simulation/Model/Model.h>

#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace osc;

namespace
{
    // maximum distance between the current commit and the "root" commit (i.e. a commit with no parent)
    inline constexpr int c_MaxUndo = 32;

    // maximum distance between the branch head and the current commit (i.e. how big the redo buffer can be)
    inline constexpr int c_MaxRedo = 32;

    std::unique_ptr<OpenSim::Model> makeNewModel()
    {
        auto rv = std::make_unique<OpenSim::Model>();
        rv->updDisplayHints().set_show_frames(true);
        return rv;
    }

    class UiModelStatePair final : public opyn::ModelStatePair {
    public:

        UiModelStatePair() :
            UiModelStatePair{makeNewModel()}
        {}

        explicit UiModelStatePair(const std::string& osim) :
            UiModelStatePair{opyn::load_model(osim)}
        {}

        explicit UiModelStatePair(std::unique_ptr<OpenSim::Model> _model) :
            m_Model{std::move(_model)},
            m_FixupScaleFactor{1.0f}
        {
            opyn::initialize_model(*m_Model);
            opyn::initialize_state(*m_Model);
        }

        UiModelStatePair(const UiModelStatePair& other) :
            m_Model{std::make_unique<OpenSim::Model>(*other.m_Model)},
            m_FixupScaleFactor{other.m_FixupScaleFactor},
            m_MaybeSelected{other.m_MaybeSelected},
            m_MaybeHovered{other.m_MaybeHovered}
        {
            opyn::initialize_model(*m_Model);
            opyn::initialize_state(*m_Model);
        }

        UiModelStatePair(UiModelStatePair&&) noexcept = default;
        UiModelStatePair& operator=(const UiModelStatePair&) = delete;
        UiModelStatePair& operator=(UiModelStatePair&&) noexcept = default;
        ~UiModelStatePair() noexcept override = default;

        const OpenSim::Model& impl_get_model() const final
        {
            return *m_Model;
        }

        OpenSim::Model& updModel()
        {
            m_ModelVersion = UID{};
            return *m_Model;
        }

        UID impl_get_model_version() const final
        {
            return m_ModelVersion;
        }

        void setModelVersion(UID version)
        {
            m_ModelVersion = version;
        }

        const SimTK::State& impl_get_state() const final
        {
            return m_Model->getWorkingState();
        }

        UID impl_get_state_version() const final
        {
            return m_ModelVersion;
        }

        float impl_get_fixup_scale_factor() const final
        {
            return m_FixupScaleFactor;
        }

        void impl_set_fixup_scale_factor(float sf) final
        {
            m_FixupScaleFactor = sf;
        }

        const OpenSim::ComponentPath& getSelectedPath() const
        {
            return m_MaybeSelected;
        }

        void setSelectedPath(const OpenSim::ComponentPath& p)
        {
            m_MaybeSelected = p;
        }

        const OpenSim::Component* impl_get_selected() const final
        {
            return opyn::find_component(*m_Model, m_MaybeSelected);
        }

        void impl_set_selected(const OpenSim::Component* c) final
        {
            m_MaybeSelected = opyn::get_absolute_path_or_empty(c);
        }

        const OpenSim::ComponentPath& getHoveredPath() const
        {
            return m_MaybeHovered;
        }

        void setHoveredPath(const OpenSim::ComponentPath& p)
        {
            m_MaybeHovered = p;
        }

        const OpenSim::Component* impl_get_hovered() const final
        {
            return opyn::find_component(*m_Model, m_MaybeHovered);
        }

        void impl_set_hovered(const OpenSim::Component* c) final
        {
            m_MaybeHovered = opyn::get_absolute_path_or_empty(c);
        }

    private:
        // the model, finalized from its properties
        std::unique_ptr<OpenSim::Model> m_Model;
        UID m_ModelVersion;

        // fixup scale factor of the model
        //
        // this scales up/down the decorations of the model - used for extremely
        // undersized models (e.g. fly leg)
        float m_FixupScaleFactor;

        // (maybe) absolute path to the current selection (empty otherwise)
        OpenSim::ComponentPath m_MaybeSelected;

        // (maybe) absolute path to the current hover (empty otherwise)
        OpenSim::ComponentPath m_MaybeHovered;
    };

    void CopySelectedAndHovered(const UiModelStatePair& src, UiModelStatePair& dest)
    {
        dest.setSelectedPath(src.getSelectedPath());
        dest.setHoveredPath(src.getHoveredPath());
    }
}

class osc::UndoableModelStatePair::Impl final {
public:

    Impl()
    {
        doCommit("created a new model");  // make initial commit
    }

    explicit Impl(const OpenSim::Model& m) :
        Impl{std::make_unique<OpenSim::Model>(m)}
    {}

    // crete a new commit graph that contains a backup of the given model
    explicit Impl(std::unique_ptr<OpenSim::Model> m) :
        m_Scratch{std::move(m)}
    {
        std::stringstream ss;
        if (auto inputPath = opyn::try_find_input_file(getModel())) {
            ss << "loaded " << inputPath->filename().string();
        }
        else {
            ss << "loaded model";
        }
        doCommit(std::move(ss).str());  // make initial commit
    }

    explicit Impl(const std::filesystem::path& osimPath) :
        Impl{opyn::load_model(osimPath)}
    {
        setUpToDateWithFilesystem(std::filesystem::last_write_time(osimPath));
    }

    bool isUpToDateWithFilesystem() const
    {
        return getCheckoutID() == getFilesystemVersion();
    }

    void setUpToDateWithFilesystem(std::filesystem::file_time_type t)
    {
        m_MaybeFilesystemTimestamp = t;
        m_MaybeCommitSavedToDisk = m_CurrentHead;
    }

    std::filesystem::file_time_type getLastFilesystemWriteTime() const
    {
        return m_MaybeFilesystemTimestamp;
    }

    ModelStateCommit getLatestCommit() const
    {
        return getHeadCommit();
    }

    bool canUndo() const
    {
        const ModelStateCommit* c = tryGetCommitByID(m_CurrentHead);
        return c ? hasCommit(c->get_parent_id()) : false;
    }

    void doUndo()
    {
        if (canUndo())
        {
            undo();
        }
    }

    bool canRedo() const
    {
        return distance(m_BranchHead, m_CurrentHead) > 0;
    }

    void doRedo()
    {
        if (canRedo())
        {
            redo();
        }
    }

    void commit(std::string_view message)
    {
        OSC_PERF("commit model");
        doCommit(message);
    }

    void rollback()
    {
        checkout();  // care: skip copying selection because a rollback is aggro
    }

    bool tryCheckout(const ModelStateCommit& commit)
    {
        if (!m_Commits.contains(commit.get_id()))
        {
            return false;  // commit isn't in this model's storage (is it from another model?)
        }

        m_CurrentHead = commit.get_id();
        checkout();
        return true;
    }

    const OpenSim::Model& getModel() const
    {
        return m_Scratch.get_model();
    }

    OpenSim::Model& updModel()
    {
        return m_Scratch.updModel();
    }

    void setModel(std::unique_ptr<OpenSim::Model> newModel)
    {
        UiModelStatePair p{std::move(newModel)};
        CopySelectedAndHovered(m_Scratch, p);
        p.set_fixup_scale_factor(m_Scratch.get_fixup_scale_factor());
        m_Scratch = std::move(p);
    }

    void resetModel()
    {
        UiModelStatePair p;
        CopySelectedAndHovered(m_Scratch, p);
        p.set_fixup_scale_factor(m_Scratch.get_fixup_scale_factor());
        m_Scratch = std::move(p);
    }

    void loadModel(const std::filesystem::path& path)
    {
        setModel(opyn::load_model(path));
        setUpToDateWithFilesystem(std::filesystem::last_write_time(path));
    }

    UID getModelVersion() const
    {
        return m_Scratch.get_model_version();
    }

    void setModelVersion(UID version)
    {
        m_Scratch.setModelVersion(version);
    }

    const SimTK::State& getState() const
    {
        return m_Scratch.get_state();
    }

    UID getStateVersion() const
    {
        return m_Scratch.get_state_version();
    }

    float getFixupScaleFactor() const
    {
        return m_Scratch.get_fixup_scale_factor();
    }

    void setFixupScaleFactor(float v)
    {
        m_Scratch.set_fixup_scale_factor(v);
    }

    const OpenSim::Component* getSelected() const
    {
        return m_Scratch.get_selected();
    }

    void setSelected(const OpenSim::Component* c)
    {
        m_Scratch.set_selected(c);
    }

    const OpenSim::Component* getHovered() const
    {
        return m_Scratch.get_hovered();
    }

    void setHovered(const OpenSim::Component* c)
    {
        m_Scratch.set_hovered(c);
    }

    std::shared_ptr<Environment> implUpdAssociatedEnvironment()
    {
        return m_Environment;
    }

private:

    UID doCommit(std::string_view message)
    {
        auto commit = ModelStateCommit{m_Scratch, message, m_CurrentHead};
        UID commitID = commit.get_id();

        m_Commits.try_emplace(commitID, std::move(commit));
        m_CurrentHead = commitID;
        m_BranchHead = commitID;

        garbageCollect();

        return commitID;
    }

    // try to lookup a commit by its ID
    const ModelStateCommit* tryGetCommitByID(UID id) const
    {
        return lookup_or_nullptr(m_Commits, id);
    }

    const ModelStateCommit& getHeadCommit() const
    {
        OSC_ASSERT(m_CurrentHead != UID::empty());
        OSC_ASSERT(hasCommit(m_CurrentHead));

        return *tryGetCommitByID(m_CurrentHead);
    }

    // try to lookup the *parent* of a given commit, or return an empty (senteniel) ID
    UID tryGetParentIDOrEmpty(UID id) const
    {
        const ModelStateCommit* commit = tryGetCommitByID(id);
        return commit ? commit->get_parent_id() : UID::empty();
    }

    // returns `true` if a commit with the given ID has been stored
    bool hasCommit(UID id) const
    {
        return tryGetCommitByID(id) != nullptr;
    }

    // returns the number of hops between commit `a` and commit `b`
    //
    // returns -1 if commit `b` cannot be reached from commit `a`
    int distance(UID a, UID b) const
    {
        if (a == b)
        {
            return 0;
        }

        int n = 1;
        UID parent = tryGetParentIDOrEmpty(a);

        while (parent != b && parent != UID::empty())
        {
            parent = tryGetParentIDOrEmpty(parent);
            n++;
        }

        return parent == b ? n : -1;
    }

    // returns a pointer to a commit that is the nth ancestor from `a`
    //
    // (e.g. n==0 returns `a`, n==1 returns `a`'s parent, n==2 returns `a`'s grandparent)
    //
    // returns `nullptr` if there are insufficient ancestors. `n` must be >= 0
    const ModelStateCommit* nthAncestor(UID a, int n) const
    {
        if (n < 0)
        {
            return nullptr;
        }

        const ModelStateCommit* c = tryGetCommitByID(a);

        if (!c || n == 0)
        {
            return c;
        }

        int i = 1;
        c = tryGetCommitByID(c->get_parent_id());

        while (c && i < n)
        {
            c = tryGetCommitByID(c->get_parent_id());
            i++;
        }

        return c;
    }

    // returns the UID that is the nth ancestor from `a`, or empty if there are insufficient ancestors
    UID nthAncestorID(UID a, int n) const
    {
        const ModelStateCommit* c = nthAncestor(a, n);
        return c ? c->get_id() : UID::empty();
    }

    // returns `true` if `maybeAncestor` is an ancestor of `id`
    bool isAncestor(UID maybeAncestor, UID id)
    {
        const ModelStateCommit* c = tryGetCommitByID(id);

        while (c != nullptr && c->get_id() != maybeAncestor)
        {
            c = tryGetCommitByID(c->get_parent_id());
        }

        return c != nullptr;
    }

    // remove a range of commits from `start` (inclusive) to `end` (exclusive)
    void eraseCommitRange(UID start, UID end)
    {
        auto it = m_Commits.find(start);

        while (it != m_Commits.end() && it->second.get_id() != end)
        {
            UID parent = it->second.get_parent_id();
            m_Commits.erase(it);

            it = m_Commits.find(parent);
        }
    }

    // garbage collect (erase) commits that fall outside the maximum undo depth
    void garbageCollectMaxUndo()
    {
        static_assert(c_MaxUndo >= 0);

        UID firstBadCommitIDOrEmpty = nthAncestorID(m_CurrentHead, c_MaxUndo + 1);
        eraseCommitRange(firstBadCommitIDOrEmpty, UID::empty());
    }

    // garbage collect (erase) commits that fall outside the maximum redo depth
    void garbageCollectMaxRedo()
    {
        static_assert(c_MaxRedo >= 0);

        int numRedos = distance(m_BranchHead, m_CurrentHead);
        int numDeletions = numRedos - c_MaxRedo;

        if (numDeletions <= 0)
        {
            return;
        }

        UID newBranchHead = nthAncestorID(m_BranchHead, numDeletions);
        eraseCommitRange(m_BranchHead, newBranchHead);
        m_BranchHead = newBranchHead;
    }

    void garbageCollectUnreachable()
    {
        std::erase_if(m_Commits, [this](const auto& p)
        {
            return !(p.first == m_BranchHead || isAncestor(p.first, m_BranchHead));
        });
    }

    // remove out-of-bounds, deleted, out-of-date, etc. commits
    void garbageCollect()
    {
        garbageCollectMaxUndo();
        garbageCollectMaxRedo();
        garbageCollectUnreachable();
    }

    // returns commit ID of the currently active checkout
    UID getCheckoutID() const
    {
        return m_CurrentHead;
    }

    // checks out the current checkout to be active (scratch)
    //
    // effectively, reset the scratch space
    void checkout()
    {
        // because this is a "reset", try to maintain useful state from the
        // scratch space - things like reset and scaling state, which the
        // user might expect to be maintained even if a crash happened

        const ModelStateCommit* c = tryGetCommitByID(m_CurrentHead);

        if (c)
        {
            UiModelStatePair newScratch{std::make_unique<OpenSim::Model>(*c->get_model())};
            CopySelectedAndHovered(m_Scratch, newScratch);
            newScratch.set_fixup_scale_factor(m_Scratch.get_fixup_scale_factor());
            m_Scratch = std::move(newScratch);
        }
    }

    // performs an undo, if possible
    //
    // effectively, checks out HEAD~1
    void undo()
    {
        const ModelStateCommit* c = tryGetCommitByID(m_CurrentHead);

        if (!c)
        {
            return;
        }

        const ModelStateCommit* parent = tryGetCommitByID(c->get_parent_id());

        if (!parent)
        {
            return;
        }

        // perform fixups to ensure the user experience is best:
        //
        // - user's selection state should be "sticky" between undo/redo
        // - user's scene scale factor should be "sticky" between undo/redo
        UiModelStatePair newModel{std::make_unique<OpenSim::Model>(*parent->get_model())};
        CopySelectedAndHovered(m_Scratch, newModel);
        newModel.set_fixup_scale_factor(m_Scratch.get_fixup_scale_factor());

        m_Scratch = std::move(newModel);
        m_CurrentHead = parent->get_id();
    }

    // performs a redo, if possible
    void redo()
    {
        int dist = distance(m_BranchHead, m_CurrentHead);

        if (dist < 1)
        {
            return;
        }

        const ModelStateCommit* c = nthAncestor(m_BranchHead, dist - 1);

        if (!c)
        {
            return;
        }

        // perform fixups to ensure the user experience is best:
        //
        // - user's selection state should be "sticky" between undo/redo
        // - user's scene scale factor should be "sticky" between undo/redo
        UiModelStatePair newModel{std::make_unique<OpenSim::Model>(*c->get_model())};
        CopySelectedAndHovered(m_Scratch, newModel);
        newModel.set_fixup_scale_factor(m_Scratch.get_fixup_scale_factor());

        m_Scratch = std::move(newModel);
        m_CurrentHead = c->get_id();
    }

    std::filesystem::path getFilesystemLocation() const
    {
        return std::filesystem::path{getModel().getInputFileName()};
    }

    void setFilesystemLocation(const std::filesystem::path& p)
    {
        updModel().setInputFileName(p.string());
    }

    UID getFilesystemVersion() const
    {
        return m_MaybeCommitSavedToDisk;
    }

    // mutable staging area that calling code can mutate
    UiModelStatePair m_Scratch;

    // where scratch will commit to (i.e. the parent of the scratch area)
    UID m_CurrentHead = UID::empty();

    // head of the current branch (i.e. "main") - may be ahead of current branch (undo/redo)
    UID m_BranchHead = UID::empty();

    // underlying storage for immutable commits
    std::unordered_map<UID, ModelStateCommit> m_Commits;

    // the timestamp of the on-disk data (needed to know when to trigger a reload)
    std::filesystem::file_time_type m_MaybeFilesystemTimestamp;

    // (maybe) the version of the model that was last saved to disk
    UID m_MaybeCommitSavedToDisk = UID::empty();

    // the `Environment` that this model is associated with
    std::shared_ptr<Environment> m_Environment = std::make_shared<Environment>();
};


// public API (PIMPL)

osc::UndoableModelStatePair::UndoableModelStatePair() :
    impl_{std::make_unique<Impl>()}
{
}

osc::UndoableModelStatePair::UndoableModelStatePair(const OpenSim::Model& model) :
    impl_{std::make_unique<Impl>(model)}
{
}

osc::UndoableModelStatePair::UndoableModelStatePair(std::unique_ptr<OpenSim::Model> model) :
    impl_{std::make_unique<Impl>(std::move(model))}
{
}

osc::UndoableModelStatePair::UndoableModelStatePair(const std::filesystem::path& osimPath) :
    impl_{std::make_unique<Impl>(osimPath)}
{
}

osc::UndoableModelStatePair::UndoableModelStatePair(const UndoableModelStatePair& src) :
    impl_{std::make_unique<Impl>(*src.impl_)}
{
}

osc::UndoableModelStatePair::UndoableModelStatePair(UndoableModelStatePair&&) noexcept = default;

UndoableModelStatePair& osc::UndoableModelStatePair::operator=(const UndoableModelStatePair& src)
{
    if (&src != this)
    {
        std::unique_ptr<Impl> cpy = std::make_unique<Impl>(*src.impl_);
        std::swap(impl_, cpy);
    }

    return *this;
}

UndoableModelStatePair& osc::UndoableModelStatePair::operator=(UndoableModelStatePair&&) noexcept = default;
osc::UndoableModelStatePair::~UndoableModelStatePair() noexcept = default;

bool osc::UndoableModelStatePair::is_up_to_date_with_filesystem() const
{
    return impl_->isUpToDateWithFilesystem();
}

std::filesystem::file_time_type osc::UndoableModelStatePair::get_last_filesystem_write_time() const
{
    return impl_->getLastFilesystemWriteTime();
}


ModelStateCommit osc::UndoableModelStatePair::get_latest_commit() const
{
    return impl_->getLatestCommit();
}

bool osc::UndoableModelStatePair::can_undo() const
{
    return impl_->canUndo();
}

void osc::UndoableModelStatePair::do_undo()
{
    impl_->doUndo();
}

bool osc::UndoableModelStatePair::can_redo() const
{
    return impl_->canRedo();
}

void osc::UndoableModelStatePair::do_redo()
{
    impl_->doRedo();
}

void osc::UndoableModelStatePair::rollback()
{
    impl_->rollback();
}

bool osc::UndoableModelStatePair::try_checkout(const ModelStateCommit& commit)
{
    return impl_->tryCheckout(commit);
}

void osc::UndoableModelStatePair::set_model(std::unique_ptr<OpenSim::Model> newModel)
{
    impl_->setModel(std::move(newModel));
}

void osc::UndoableModelStatePair::reset_model()
{
    impl_->resetModel();
}

void osc::UndoableModelStatePair::load_model(const std::filesystem::path& p)
{
    impl_->loadModel(p);
}

void osc::UndoableModelStatePair::impl_set_model_version(UID version)
{
    impl_->setModelVersion(version);
}

const OpenSim::Model& osc::UndoableModelStatePair::impl_get_model() const
{
    return impl_->getModel();
}

const SimTK::State& osc::UndoableModelStatePair::impl_get_state() const
{
    return impl_->getState();
}

OpenSim::Model& osc::UndoableModelStatePair::impl_upd_model()
{
    return impl_->updModel();
}

void osc::UndoableModelStatePair::impl_commit(std::string_view commitMessage)
{
    impl_->commit(commitMessage);
}

UID osc::UndoableModelStatePair::impl_get_model_version() const
{
    return impl_->getModelVersion();
}

UID osc::UndoableModelStatePair::impl_get_state_version() const
{
    return impl_->getStateVersion();
}

float osc::UndoableModelStatePair::impl_get_fixup_scale_factor() const
{
    return impl_->getFixupScaleFactor();
}

void osc::UndoableModelStatePair::impl_set_fixup_scale_factor(float v)
{
    impl_->setFixupScaleFactor(v);
}

const OpenSim::Component* osc::UndoableModelStatePair::impl_get_selected() const
{
    return impl_->getSelected();
}

void osc::UndoableModelStatePair::impl_set_selected(const OpenSim::Component* c)
{
    impl_->setSelected(c);
}

const OpenSim::Component* osc::UndoableModelStatePair::impl_get_hovered() const
{
    return impl_->getHovered();
}

void osc::UndoableModelStatePair::impl_set_hovered(const OpenSim::Component* c)
{
    impl_->setHovered(c);
}

std::shared_ptr<Environment> osc::UndoableModelStatePair::impl_upd_associated_environment() const
{
    return impl_->implUpdAssociatedEnvironment();
}

void osc::UndoableModelStatePair::impl_set_up_to_date_with_filesystem(std::filesystem::file_time_type t)
{
    impl_->setUpToDateWithFilesystem(t);
}
