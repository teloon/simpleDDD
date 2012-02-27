# Introduction 
"simpleDDD" is short for "simple Duplicate Document Detectio". This program is a simple implementation of the model in paper "[Finding similar files in large document repositories][1]". For the details of this algorithm, you can read that paper.

[1]: http://dl.acm.org/citation.cfm?id=1081916

# System design
![design](https://github.com/teloon/simpleDDD/blob/master/doc/desineV2.png?raw=true)

# Environment requirement
__Platform__:

>Linux(originally on ubuntu 8.04)

__dependency__:

>1. __tokyo__ cabinet：[how to install][tc]
>2. the __filesystem__ module of boost 1.39: [how to install][boost]
>3. __gcc__, __g++__: you should change the makefile according to the version of gcc you use, i.e. change the "__43__" in "-lboost_filesystem-gcc43-mt" to the version number of your gcc
>4. __python2.5__ and __python-dev__
>5. __libssl-dev__

[tc]: http://tokyocabinet.sourceforge.net/spex-en.html#installation
[boost]: http://www.boost.org/doc/libs/1_39_0/more/getting_started/unix-variants.html#easy-build-and-install

#Parameters
There're mainly 11 parameters you can adjust according to your application. They are defined in the file "glo.h".
#### THRESHOLD
the threshold of duplicate documents. It determines how strict you want the "duplicate" be.T
#### resultFileName
As the name indicate, it defines the name of result file
#### TMIN and TMAX
They are used in the [TTTD] algorithm. They define the minimum size and maximum size of the chunk respectively. They're crucial parameters, which determine the granularity of the chunking. The larger these two parameters are, the smaller the precision will be, and the faster it will be.
#### D and DDASH
They are two divisors used in [TTTD] algorithm. According to the experiment, they have little impact on the result.
#### ENABLE_FITTING
It defines whether to check "can the metadata fit in memory" or not. If your data is so large that the metadata file can't fit in memory, you need do partition. If small, you don't need partition(because it costs more time). If this flag is false, the program does partition all the time.
#### SLIDING_WINDOW_SIZE
The size of the sliding window in [TTTD]. The smaller it is, the smaller the precision will be and the faster it will be.
#### DEBUG_LEVEL
Check file "debug.h"
#### MAX_FILE_SIZE
The maximum size of the sub-problem file. 512*1024 for default.
#### sourceDir
The name of the directory where the corpus locate.

[TTTD]: http://www.hpl.hp.com/techreports/2005/HPL-2005-30R1.html

# Running
1. Install all the dependency

2. Adjust all the parameters that needs for changing. Then run `./bat.sh` in the src directory. 

3. Run `./main`.

# Debugging
Change the value of parameter "DEBUG_LEVEL" and run `./bat_debug.sh` to compile. Then you can debug in one of the following ways:

>1. Set the "DEBUG_LEVEL", and different levels of debug information will be printed according to it. (Redirection is recommended, because of possibly massive debug info)

>2. Run `gdb main`.

>3. profiling: run `./main`. After the program ends, run `gprof gmon.out`.
