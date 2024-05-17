{-  
Login:  xzvara01
Author: Adam Zvara
Date:   3/2024  
Brief:  Main entrypoint of program, parses arguments and controls the subtasks
-}

import System.Environment (getArgs)
import System.IO.Error (isUserError, ioeGetErrorString)
import System.Exit (exitWith, ExitCode (ExitFailure))
import Control.Exception (catch)
import Control.Monad (when)
import Data.Maybe (catMaybes)

import Datafile (DataValue(..), parseDataFile)
import DecisionTree (parseDecisionTree, traverseTree)
import Cart (createTreeCART)

main :: IO ()
main = do
    -- Parse arguments, get task and execute it
    task <- getNthArg 0
    case task of 
        "-1"     -> classificationTask
        "-2"     -> trainingTask
        "-h"     -> printUsage
        "--help" -> printUsage
        _        -> ioError $ userError "Invalid parameter detected"
    `catch` generalErrHandler

-- General exception handler which catches user errors and prints them
-- otherwise it rethrows the error
generalErrHandler :: IOError -> IO a
generalErrHandler e 
    | isUserError e = do putStrLn (ioeGetErrorString e)
                         exitWith $ ExitFailure 1
    | otherwise = ioError e

printUsage :: IO ()
printUsage = do
    putStrLn "Usage: ./flp-run -MODE [OPTIONS ...]"
    putStr "  ./flp-run -1 dec_tree_file data_assign_file"
    putStrLn " \t Use decision tree to assign labels to data"
    putStr "  ./flp-run -2 data_train_file"
    putStrLn " \t\t\t Create new decision tree based on given data"

-- Parse Nth argument from cmdline (indexing from 0)
getNthArg :: Int -> IO String
getNthArg n = do
    args <- getArgs
    when (length args < n + 1) $ ioError $ userError "Not enough arguments provided"
    return $ args !! n

{------------------------- Classification --------------------------}

-- First task: load decision tree and data from files, classify data
classificationTask :: IO ()
classificationTask = do
    -- Read and parse decision tree
    treeFileName <- getNthArg 1 `catch` generalErrHandler
    treeContents <- readFile treeFileName
    tree <- case parseDecisionTree treeContents of
         Left err_msg -> ioError $ userError $ "Tree could not be parsed: " ++ err_msg
         Right tree -> return tree

    -- Read and parse data file
    dataFileName <- getNthArg 2 `catch` generalErrHandler
    dataContents <- readFile dataFileName
    let data' = parseDataFile False dataContents
    x <- case data' of
        Nothing -> ioError $ userError "Data file is not correctly formatted"
        Just x' -> return $ map features x'

    -- For each input look through decision tree and find its class
    let classes = map (traverseTree tree) x
    if Nothing `elem` classes then
        ioError $ userError "Tree index out of range for given data"
    else
        mapM_ putStrLn $ catMaybes classes

{----------------------------- Training ------------------------------}

-- Second task: load training data from file and construct decision tree 
-- based on CART algorithm with Gini index
trainingTask :: IO ()
trainingTask = do
    -- Read and parse data file
    dataFileName <- getNthArg 1 `catch` generalErrHandler
    dataContents <- readFile dataFileName
    let data' = parseDataFile True dataContents

    x <- case data' of
        Nothing -> ioError $ userError "Data file is not correctly formatted"
        Just x' -> return x'

    -- Create and print decision tree based on CART algorithm
    let maxDepth = -1 -- No limit on the depth
        tree = createTreeCART maxDepth x
        
    print tree 