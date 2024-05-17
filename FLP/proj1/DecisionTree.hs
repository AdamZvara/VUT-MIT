{-  
Login:  xzvara01
Author: Adam Zvara
Date:   3/2024  
Brief:  DecisionTree implementation
-}

module DecisionTree ( 
    DecisionTree (..), 
    parseDecisionTree, 
    traverseTree
) where

import Data.List (intercalate, elemIndex, isPrefixOf)
import Data.Maybe (fromMaybe)
import Data.Char (isSpace)

type Class = String
-- Data type representing decision tree, which either contains Leaf with class string
-- or Node with index as Integer, threshold as Float, left and right subtree
data DecisionTree = Leaf Class | Node Int Float DecisionTree DecisionTree

-- Make DecisionTree showable
instance Show DecisionTree where
    show (Leaf c) = show c
    show (Node i v leftSub rightSub) = showDecTree (Node i v leftSub rightSub) 0

-- Show the (sub) DecisionTree with correct indentation level 
showDecTree ::  DecisionTree -> Int -> String
showDecTree (Leaf c) level = "\n" ++ indent ++ leafString ++ c
    where indent = replicate (level * 2) ' '
          leafString = "Leaf: "
showDecTree (Node i v l r) level = 
    start ++ indent ++ nodeString ++ show i ++ separator ++ 
    show v ++ showDecTree l newLevel ++ showDecTree r newLevel
    where newLevel = level + 1
          indent = replicate (level * 2) ' '
          nodeString = "Node: "
          separator = ", "
          start = if level /= 0 then "\n" else ""

{------------------------- DecisionTree parsing --------------------------}

-- Parse the decision tree
-- The first argument is the decision tree in "raw" string (direcly read from file)
-- which is also the way how the subtrees are passed into other parsing functions
parseDecisionTree :: String -> Either String DecisionTree
parseDecisionTree rawTreeString
    | "Leaf" `isPrefixOf` rawTreeString =
        if length treeLines /= 1 then -- Check if there are no more subtrees below leaf
            Left "Incorrectly ended leaf (has elements under it)"
        else 
            Right (Leaf (getLeafClass treeString'))
    | otherwise = 
        -- Split remaining treeString into left and right subtrees and parse them recursively
        case splitToSubtrees treeString' of 
            Left err -> Left err -- If any error occured when parsing tree, return it
            Right (leftSubTree, rightSubTree) -> createNode root leftSubTree rightSubTree
    where treeLines = filter (not . all isSpace) . lines $ rawTreeString -- Remove lines with only whitespace chars
          treeString' = unlines treeLines
          root = head treeLines

-- Split tree in raw string format into subtrees (in the same raw format)
-- 
-- The splitting is done based on the indentation -> after root node is parsed, the next tree level
-- is found based on 2 spaces indentation 
-- 
-- ! Warning ! when using `tab` to indent levels make sure, that each tab represents 
-- 2 spaces in your code editor and not more (or less)!
splitToSubtrees :: String -> Either String (String, String)
splitToSubtrees tree = 
    -- If there are not exactly 2 items (Node or Leaf) in the next level or some other error occured, return err
    if (length nextLevelIndent /= 2) || (rightChildIdx == -1) then
        Left "Incorrect tree indentation detected"
    else 
        Right (leftSubTree, rightSubTree)
    where tabsFreeTree = map tabsToSpaces . lines $ tree -- Convert tabs to (2) spaces in tree, split into lines
          children = drop 1 tabsFreeTree  -- Drop the root

          -- Split the tree into 2 subtrees based on identation level (2 spaces at the beginning of new level)
          nextLevelIndent = filter startsWithTwoSpaces tabsFreeTree
          rightChild = last nextLevelIndent
          -- drop the first item (start of leftsubtree) because if the left and right subtrees exatcly match in first element
          -- elemIndex returns the first one
          rightChildIdx = fromMaybe (-1) (elemIndex rightChild (drop 1 children)) + 1

          -- drop 2 because we are dropping the next identation level (2 spaces) for each line of the subtree
          -- so it can be parsed recursively in the next call
          leftSubTree = unlines . map (drop 2) . take rightChildIdx $ children 
          rightSubTree = unlines . map (drop 2) . drop rightChildIdx $ children

-- Create node with parsed values and add left and right subtrees, propagate errors to caller
createNode :: String -> String -> String -> Either String DecisionTree
createNode root leftSub rightSub = 
    case (parseDecisionTree leftSub, parseDecisionTree rightSub) of 
        (Right left, Right right) -> Right (Node idx val left right) -- All good, make new node
        -- If either recursive call failed, return error message
        (Left err, _) -> Left err
        (_, Left err) -> Left err
    where (idx, val) = getNodeVals root

-- Get the index and value from Node
getNodeVals :: String -> (Int, Float)
getNodeVals node = (read idx, read val)
    where splittedNode = words node
          idx = init $ splittedNode !! 1 -- init because we want to drop the "," symbol after the index
          val = splittedNode !! 2

-- Parse the class atribute from Leaf
getLeafClass :: String -> Class
getLeafClass = unwords . tail . words . head . lines

{------------------------- DecisionTree traversing --------------------------}

-- Traverse DecisionTree using list of floats as values to compare with Node threshold
traverseTree :: DecisionTree -> [Float] -> Maybe Class
traverseTree (Leaf c) _ = Just c
traverseTree (Node idx val left right) vals
  | idx > length vals = Nothing
  | vals !! idx <= val = traverseTree left vals
  | otherwise = traverseTree right vals

{-------------------------- General functions ---------------------------}

-- Predicate if string starts with (only) two spaces
startsWithTwoSpaces :: String -> Bool
startsWithTwoSpaces [] = False
startsWithTwoSpaces [_] = False
startsWithTwoSpaces (x:xs:xss) = x == ' ' && xs == ' ' && head xss /= ' '

-- Convert tabs into 2 whitespaces
tabsToSpaces :: String -> String
tabsToSpaces w = intercalate "" (map (\x -> if x == "\t" then replicate 2 ' ' else x) charList)
    where charList = map (:[]) w