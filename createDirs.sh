
#!/bin/bash

# Create dirA
mkdir dirA
touch dirA/A
touch dirA/B
touch dirA/C.txt

mkdir dirA/dir1
touch dirA/dir1/A1

mkdir -p dirA/dir2/dir21
mkdir -p dirA/dir2/dir22
touch dirA/dir2/dir22/file.csv

mkdir dirA/dir3

# Create dirB
mkdir dirB
touch dirB/B
touch dirB/C.txt

mkdir dirB/dir1
touch dirB/dir1/A1
mkdir -p dirB/dir1/dir11/dir111/dir1111
touch dirB/dir1/dir11/dir111/dir1111/test.txt

mkdir -p dirB/dir2/dir21
mkdir -p dirB/dir2/dir22
touch dirB/dir2/dir22/file.csv
touch dirB/dir2/myfile.c

# Create dirC
mkdir dirC

mkdir -p dirC/dir1/dir11
ln -s ../../link3 dirC/dir1/dir11/link2
ln -s dir11/link2 dirC/dir1/link1

mkdir -p dirC/dir2
touch dirC/dir2/file.c
ln -s ../dir3/my_file.txt dirC/dir2/link5
ln -s file.c dirC/dir2/link6

mkdir -p dirC/dir3
touch dirC/dir3/my_file.txt

ln -s dir2/file.c dirC/link3
ln -s dir2/link5 dirC/link4

# Create dirD
mkdir dirD

mkdir -p dirD/dir1/dir11
ln -s ../../link3 dirD/dir1/dir11/link2
ln -s dir11/link2 dirD/dir1/link1

mkdir -p dirD/dir2
touch dirD/dir2/file.c
ln -s /home/zenki/Documents/DIT/OS/4/DelisOS_P4/dirD/dir2/file.c dirD/dir2/link6

mkdir -p dirD/dir3
touch dirD/dir3/my_file.txt

ln -s dir2/file.c dirD/link3
ln -s dir3/my_file.txt dirD/link4

#Create dirHardA
mkdir dirHardA
touch dirHardA/A
ln dirHardA/A dirHardA/B
ln dirHardA/A dirHardA/C
ln dirHardA/A dirHardA/D

#Create dirHardB
mkdir dirHardB
touch dirHardB/A
ln dirHardB/A dirHardB/F
ln dirHardB/A dirHardB/G

