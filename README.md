# MIT 6.1810 Lab
This repository is a track of process I walked through during my study of the MIT 6.1810 (Operating System Engineering) Lab, which is made public in [this website](https://pdos.csail.mit.edu/6.1810/2025/index.html).

# The Structure
## Folders
The `xv6-OS` folder under the root folder is the exact cloned copy of the official, cloned from:

```
git clone git://g.csail.mit.edu/xv6-labs-2025
```

except git files are moved out to the root directory of this repository.

In addition, the `.gitignore` file is edited to ignore the `.DS_STORE` file on MacOS.

## Branches

The repository holds a few branches, each correspond to one lab (lab utilities, lab systemcall, etc.). You can view the result of lab and process by switching branches. For each branch, only the `xv6-OS` folder should change.