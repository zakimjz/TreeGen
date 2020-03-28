# TreeGen Algorithm

Contains the synthetic tree generator described in [2005-treeminer:tkde]

**Relevant Publications**

* [2005-treeminer:tkde] Mohammed J. Zaki. Efficiently mining frequent trees in a forest: algorithms and applications. IEEE Transactions on Knowledge and Data Engineering, 17(8):1021–1035, August 2005. special issue on Mining Biological Data. doi:10.1109/TKDE.2005.125.

# HOW TO

1) type tree_gen to see the options.

        -d : Depth, default 5
        -f : Fan out factor, default 5
        -a : ascii output flag
        -b : use random subtree root as starting root
        -n : Number of items, default 10
        -m : Total number of nodes in parent tree
        -o : out file name (required)
        -p : output parent tree as database
        -s : seed for random number gen
        -t : Number of subtrees, default 100      

The program first generates a master tree with total m nodes with avg
depth d and fanout f from n items (or labels)

2) you can output this parent tree in the file name specified
by -o (and with -a if you want ascii output), by specifying -p option

3) you can generate t subtrees from the parent by omitting -p option
and specifying the file name -o (with -a for ascii).

example run:

    tree_gen -f 6 -d 10 -n 100 -m 1000 -o xxx -a -t 1000

to produce a pool of 1000 trees from the master.

If you want to know the master tree, run:

    tree_gen -f 6 -d 10 -n 100 -m 1000 -o xxx.master -a -t 1000 -p


