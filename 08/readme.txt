I made implementations in Haskell and Java7.
The Java version is more elaborate and also generates dot files, but the Haskell one also works.
Both versions use a multimap for storing the connected nodes for each node.
The conflict detection is in O(#operations^2), creation of the graph in O(#conflicts).
The cycle detection starts a DFS for each transaction and returns once a back edge to an already visited node has been found. This is in O(#transactions^2).

-- Running:

cd java/bin && java Scheduler ../../schedule.txt

or

cd haskell && runhaskell Scheduler.hs ../schedule.txt