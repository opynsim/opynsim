The Release Process
===================

This is a rough overview of OPynSim's release process - mostly intended
as internal documentation for the development team.

OPynSim aims to have a  1-3 month release cadence - even if no major
changes occur in a release window. This ensures that at least one commit
of OPynSim's source code is tested, libASANed, linted, manually-checked, etc.
every 1-3 months.

Release Checklist (Markdown)
----------------------------

This release checklist is in markdown so that it can be copied + pasted into
websites like GitHub.

.. code:: markdown

	## Prepare, Test, and Build

	- [ ] Create an issue called something like `Release XX.xx.pp`
	- [ ] Copy this checklist into it
	- [ ] Bump the VERSION tag in the top-level `CMakeLists.txt` file to `XX.xx.pp`
	- [ ] On a Linux machine, clean-build the `ErrorCheck` configuration `cd third_party/ && cmake --workflow --preset ErrorCheck && cd - && cmake --workflow --preset ErrorCheck`
	- [ ] Ensure the `ErrorCheck` build + tests all pass
	- [ ] Manually spot-check any new changes with the `ErrorCheck` build
	- [ ] Fix all bugs/problems/lints found during the above steps
	- [ ] Commit the fixes to CI and ensure CI passes
	- [ ] Download CI artifacts for all supported platforms. Install them on all applicable test/development machines. Manually spot-check that they install, work, etc.
	- [ ] Update  `CHANGELOG.md`:
	  - [ ] Move sections around such that `Unreleased` becomes `XX.xx.pp`, add a new (empty) `Unreleased` section at the top.
	  - [ ] Write a very short summary paragraph at the top of the `XX.xx.pp` section, for use in GitHub, website, etc.
	- [ ] Tag+push the `CHANGELOG.md` update commit as a release (e.g. "Update CHANGELOG.md for XX.xx.pp")
	- [ ] Rebase any active branches onto the `main` branch so that all branches are at-least compatible with the latest release, or delete the branches if they are stale.
	- [ ] Collect all build artifacts:
	  - [ ] Wheels (``.whl``) for each supported platform
	  - [ ] Built documentation, packaged into a ``.tar.gz`` file
	  - [ ] Source code, packaged into a ``.tar.gz`` file

	## Publish Release

	- [ ] Create a GitHub release from the tagged commit
	  - [ ] Upload all artifacts against it
	  - [ ] Copy + paste the release summary paragraph from `CHANGELOG.md` as the release description
	- [ ] TODO: Update Zenodo with the release
	- [ ] TODO: Update `README.md` with Zenodo release details
	- [ ] TODO: Ensure the entire repository is pushed to the official TU Delft mirror
	- [ ] TODO: Ensure all build artifacts are uploaded to `files.opynsim.eu/releases`
	- [ ] TODO: Upload built documentation to `docs.opynsim.eu`
	- [ ] TODO: Ensure repo syncs with research-software-directory.org

	## Announce Release

	- [ ] Make short announcement posts on social media:
	  - [ ] LinkedIn
	  - [ ] Bluesky
	  - [ ] SimTK

	- [ ] Ensure any collaborators/researchers involved with the release cycle are
	      informed of the release.
