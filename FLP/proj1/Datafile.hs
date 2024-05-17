{-  
Login:  xzvara01
Author: Adam Zvara
Date:   3/2024  
Brief:  Parsing training/classification data files
-}

module Datafile (
    DataValue(..),
    parseDataFile,
    getClasses
) where

import Data.List (nub)
import Data.Char (isSpace)
import Data.List.Split (splitOn)

-- The data type used to represent data read from files used for training or classification
-- It contains list of floats = features and string, which is potentially 
-- a class label (in subtask1 no class label is needed, so it is not used there and is empty)
data DataValue = DataValue {features :: [Float], classLabel :: String} deriving (Eq)

-- Parse contents of the file into data value
-- If each line has different number of features, return Nothing
parseDataFile :: Bool -> String -> Maybe [DataValue]
parseDataFile parseClass content = 
    if null parsedLines || featureLengths /= 1 then
        Nothing
    else
        Just parsedLines
    where contentLines = filter (not . all isSpace) . lines $ content -- Filter out empty lines
          parsedLines = map (parseDataValue parseClass) contentLines
          featureLengths = length $ nub $ map (length . features) parsedLines

-- Parse single line from data file
-- If parseClass is true, the last value each line is treated as a class of the object
-- and is stored in the DataValue, otherwise "" is stored
parseDataValue :: Bool -> String -> DataValue
parseDataValue parseClass line = DataValue floatList class'
    where lineSplit = splitOn "," line 
          class' = if parseClass then last lineSplit else ""
          floatList = map read (if parseClass then init lineSplit else lineSplit) 

-- Function to retrieve classes from list of DataValues
getClasses :: [DataValue] -> [String]
getClasses = map classLabel