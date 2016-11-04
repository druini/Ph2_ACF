The Ph2_ACF git repository is here 
https://gitlab.cern.ch/cms_tk_ph2/Ph2_ACF.git

While the outertracker one is here:
ssh://p-cmsoutertracker@cdcvs.fnal.gov/cvs/projects/cmsoutertracker

in order to add a repository for git check here:
https://git-scm.com/book/en/v2/Git-Basics-Working-with-Remotes

Few basic commands:
git remote -v
git remote add origin ssh://p-cmsoutertracker@cdcvs.fnal.gov/cvs/projects/cmsoutertracker
git remote add cactus https://gitlab.cern.ch/cms_tk_ph2/Ph2_ACF.git

git push origin develop
git remote show origin
git pull cactus develop
git remote rm origin
