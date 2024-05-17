{-  
Login:  xzvara01
Author: Adam Zvara
Date:   3/2024  
Brief:  Implementation of createTreeCART algorithm with Gini index metric

This implementation has been inspired by article:
https://towardsdatascience.com/under-the-hood-decision-tree-454f8581684e
-}
    
module Cart ( 
    createTreeCART
) where

import Data.List (nub, sortBy, elemIndex, (\\), sort, group, maximumBy)
import Data.Ord (comparing)
import Data.Maybe (fromMaybe)

import Datafile (DataValue(..), getClasses)
import DecisionTree (DecisionTree(..))

{------------------------- Gini index of impurity --------------------------}

-- Gini index for list of items
-- GI = 1 - SUM(p^2)
giniIndex :: (Eq a) => [a] -> Float
giniIndex classes = 1.0 - sum'
    where counts = map (`countItemInList` classes) . nub $ classes
          counts' = map fromIntegral counts
          p = map (/ sum counts') counts'
          sum' = sum $ map (** 2) p

-- Gini index for pair of lists with weighted average
giniGroups :: (Eq c) => ([(a, c)], [(b, c)]) -> Float
giniGroups (g1, g2) = (giniIndex group1 * lenG1  + giniIndex group2 * lenG2) / overallLength
    where group1 = map snd g1
          group2 = map snd g2
          lenG1 = fromIntegral $ length group1 
          lenG2 = fromIntegral $ length group2
          overallLength = fromIntegral $ length group1 + length group2

-- How many times does item appear in a list
countItemInList :: (Eq a) => a -> [a] -> Int
countItemInList _ [] = 0
countItemInList x list  = length $ filter (== x) list

{--------------- Creating DecisionTree based on CART method ----------------}

-- Create DecisionTree from list of parsed DataValues based on CART method using Gini index
-- The algorithm stops when the maximum depth is reached or all the data belong to the same class
-- Negative maxDepth means there is no limit on the depth
createTreeCART :: Int -> [DataValue] -> DecisionTree
createTreeCART maxDepth x = 
    if checkIfEndCART maxDepth x then
        Leaf (mostFreqClass uniqueClasses)
    else 
        Node bestIdx bestVal (createTreeCART (maxDepth - 1) left) (createTreeCART (maxDepth-1) right)
    -- `debug` show (left, right)
    where classes = getClasses x
          uniqueClasses = nub classes
          baseGI = giniIndex classes -- Gini Index before splitting by different features and thresholds

          -- For all features calculate the best threshold using GI, then pick the best one
          featuresCount = [0 .. (length . features . head $ x) - 1]
          candidateFeatures = map (($ x) . getBestFeature baseGI) featuresCount
          bestFeature = foldl (\acc i -> if thrd i > thrd acc then i else acc) (0, 0, 0) candidateFeatures
          (bestIdx, bestVal, _) = bestFeature

          -- Split the values into left and right "subtrees" 
          left = filter (\i -> features i !! bestIdx <= bestVal) x
          right = x \\ left

-- Check if the CART algorithm should end
checkIfEndCART :: Int -> [DataValue] -> Bool
checkIfEndCART depth x 
    | depth == 0 = True -- If max depth is reached
    | areFromSameClass x = True -- If all the data belong to the same class
    | otherwise = False

areFromSameClass :: [DataValue] -> Bool
areFromSameClass x = length (nub $ getClasses x) == 1

mostFreqClass :: (Ord a) => [a] -> a
mostFreqClass = head . maximumBy (comparing length) . group . sort

{------------------------- Feature selection --------------------------}

-- Feature description: (Index of feature, threshold, Information gain)
type Feature = (Int, Float, Float)

-- Get the best threshold for a feature 
getBestFeature :: Float -> Int -> [DataValue] -> Feature
getBestFeature base featureIdx x = 
    if length thresholds <= 1 then -- If there is only one threshold, return zero information gain
        (featureIdx, head thresholds, 0)
    else
        (featureIdx, thresholdAvg !! bestFeatureIdx, infGain !! bestFeatureIdx)
    where -- Discard other features from data and sort the data by the feature
          featureClasses = zip (map ((!! featureIdx) . features) x) (getClasses x)
          sortedData = sortBy (comparing fst) featureClasses

          -- Calculate the thresholds and their Gini index
          thresholds = nub $ map fst sortedData
          -- Instead of using the threshold, use the average of two consecutive thresholds
          thresholdAvg = zipWith (\a b -> (a + b) / 2) thresholds (tail thresholds)
          thresholdsFuns = map splitToGroups thresholdAvg
          -- Calculate the Gini index for each threshold
          splits = map (\f -> f sortedData) thresholdsFuns
          infGain = map ((base-) . giniGroups) splits

          -- Get the best threshold based on Gini index
          bestFeatureIdx = fromMaybe (-1) $ elemIndex (maximum infGain) infGain

-- Split the SORTED list of pairs into two lists based on the threshold
splitToGroups :: Ord a => a -> [(a, b)] -> ([(a, b)], [(a, b)])
splitToGroups thr = span (\x -> fst x <= thr)

thrd :: (a, b, c) -> c
thrd (_,_,z) = z