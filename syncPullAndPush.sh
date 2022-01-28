#!/bin/bash

#
# Switch to ${MY_BRANCH} and ensure it is up to date with ${UPSTREAM_BRANCH_1} and ${UPSTREAM_BRANCH_2}
#
#

# Echo and Explain what is happening.
# If fails, abort.

set -e -x -u

# Update MY_BRANCH to your branch
MY_BRANCH=develop
UPSTREAM_BRANCH_1=develop #Upstream Branch
UPSTREAM_BRANCH_2=develop #Next upstream branch

set +x
echo ""
echo "################################################################"
echo "# At the latest of ${MY_BRANCH}"
set -x

git checkout ${MY_BRANCH}
git pull --prune
git submodule foreach "git checkout ${MY_BRANCH} || git checkout ${UPSTREAM_BRANCH_1} || git checkout ${UPSTREAM_BRANCH_2}"
git submodule foreach git pull --prune

set +x
echo ""
echo "################################################################"
echo "# Merging latest"
set -x

# Merge origin/${UPSTREAM_BRANCH_2} and origin/${UPSTREAM_BRANCH_1}
git submodule foreach "git merge origin/${UPSTREAM_BRANCH_1} -m \"Merging origin/${UPSTREAM_BRANCH_1}\n\n${BUILD_TAG}\" || echo OK"
git submodule foreach "git merge origin/${UPSTREAM_BRANCH_2} -m \"Merging origin/${UPSTREAM_BRANCH_2}\n\n${BUILD_TAG}\" || echo OK"

set +x
echo ""
echo "#########################################################################"
echo "NOTE! We are now at latest"
echo "IMPORTANT! If there was a merge, we are now pushing changes"
set -x

git submodule foreach git push

git merge origin/${UPSTREAM_BRANCH_1} -m "Merging origin/${UPSTREAM_BRANCH_1}\n\n${BUILD_TAG}"
git merge origin/${UPSTREAM_BRANCH_2} -m "Merging origin/${UPSTREAM_BRANCH_2}\n\n${BUILD_TAG}"
git push

echo Done
