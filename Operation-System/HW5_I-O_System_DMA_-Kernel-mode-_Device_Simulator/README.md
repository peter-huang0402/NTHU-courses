[Attached Files]
ioc_hw5.h: ioctl command labels
Makefile: insert module and remove module routine
mkdev.sh: shell script to make device node
rmdev.sh: shell script to remove device node
test.c: user mode test program


[Your Work]
implement your own module in a c file


[Test Flow]
please confirm your module has no problem in followed flow
make --> ./test --> make clean


[Grading Policy]
We will test 3 test cases, you can get 15 points by passing cases.

If your code has problems followed, you will not get any point.
compile error and warning, kernel panic, system crash
output message miss some items in page 9
printk message not add label
cannot execute the test flow by Makefile
cannot remove module normally


[Summit Format]
Please put all of your source codes, test.c, Makefile and README.txt into a folder named OS_homework5, compress the folder OS_homework5 into  OS_homework5. tar.gz, and upload OS_homework5. tar.gz to iLMS system.
If you write the bonus, please write down the interrupt number you chooses in README.txt and describe you test by SSH remote or on web interface.
Don't copy others work, or both of you will get 0 points.


[Attention]
Please confirm your module can be used on VM of SSCloud. We will test on VM.If your module fail to execute, we don’t test it again in other platform.
Please check all your code is in the compressed file and write the correct path in Makefile. We won't modify Makefile for you.
Don't summit if your code is non-completed or may crash the system. It will delay the demo progress of everyone, and you won't get any points.
We don’t accept any incorrect format in this homework. If you don’t follow our rules, you will get 0 points.
