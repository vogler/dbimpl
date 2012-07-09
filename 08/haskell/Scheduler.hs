import System
import List
import Text.Regex.Posix
import qualified Data.Map as M
import Debug.Trace

main = do args <- getArgs
          if length args < 1 then putStrLn "Usage: runhaskell Scheduler.hs scheduleFile" else process (head args)

process file = do x <- readFile file
                  let ops = map parse $ lines x
                  let con = conflicts ops
                  putStrLn "-- Conflicts:"
                  mapM_ (putStrLn.toStringConflict) $ con
                  putStrLn ""; putStrLn "-- Serializability graph:"
                  putStrLn $ serGraph con
                  putStrLn ""; putStrLn "-- Cycles:"
                  putStrLn $ cycles con

parse line = let p = "(r|w)([0-9]+)([a-zA-Z]+)" in
    let rw:t:d:[] = tail $ head $ (line =~ p :: [[String]]) in (rw, t, d)

conflicts ops = let zops = zip ops [1..] in [(a,b) | (a@(arw,at,ad),ai) <- zops, (b@(brw,bt,bd),bi) <- zops, 
                ai<bi, ad==bd, at/=bt, not (arw=="r" && brw=="r")]

serGraph xs = unlines $ "digraph SerializabilityGraph {":
              (map (\(a,b) -> "\t" ++ toStringOp a ++ " --> " ++ toStringOp b) xs)
              ++["}"]

cycles xs = -- let m = foldl (\m ((_,a,_),(_,b,_)) -> M.insertWith (++) a [b] m) M.empty xs in
            let m = M.fromListWith (++) (map (\((_,a,_),(_,b,_)) -> (a,[b])) xs) in -- M.showTree m
            let cycles = dropWhile null $ map (recCycle m []) (M.keys m) in
            if null cycles then "no cycles" else toStringCycle $ head cycles
el (Just x) = x; el Nothing  = []
recCycle m acc x | trace ("recCycle m " ++ show acc ++ " " ++ show x ++ " -> " ++ show (el $ M.lookup x m)) False = undefined
recCycle m acc x = let lacc = acc++[x] in let (v,nv) = partition (flip elem lacc) (el $ M.lookup x m) in -- m M.! x
                   if not (null v) then lacc++[head v] else let rec = dropWhile null $ map (recCycle m lacc) nv in
                   if null rec then [] else head rec

toStringCycle = concat.(List.intersperse " -> ")
toStringConflict (a,b) = toStringOp a ++ " < " ++ toStringOp b
toStringOp (rw,t,d) = rw++t++d