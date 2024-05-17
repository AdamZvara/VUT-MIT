"""
API for feature extraction from raw log lines

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import pandas as pd
import numpy as np
from collections import Counter
from scipy.special import expit

from .utils import Log
from .FeatureExtractionModels.SessionWindow import SessionBasedExtraction
from .FeatureExtractionModels.TimeWindow import TimeBasedExtraction

class FeatureExtraction(Log):
    """ Split loaded dataset into windows and extract features from them (vectorization)
    
    ### Args:
        event_col (str): name of the column containing the event id
        logging (bool): enable logging
        
    ### Notes:
        Each event template should be in format E+number -> e.g "E1", "E2", "E3", ...
    """
    
    def __init__(self, event_col, logging = True):
        super().__init__(self.__class__.__name__, logging)
        self.event_col = event_col
        self.logging = logging
        
        self.events = None
        self.extraction = None
        self.extraction_params = None
        
        # Store applied weightings
        self.tf_idf = False
        self.contrast_w = False
        
        # Temporary solution to store sessionIDs which are later printed out after anomaly detection
        self.session_ids = None
        
    def session_windowing(self, x_data, session_reg, session_col, y_data = None): 
        """ Split the log sequence into sessions based on session id found in the log message 
        
        ### Args:
            x_data (DataFrame): raw log lines
            session_reg (regex): regular expression to extract session id from log message
            session_col (str): name of the column containing the log message
            y_data (Numpy array): (optional) labels for the raw log lines
            
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids, each row represents a session
            Y (Array): array containing labels for each session (in order of appearance in the log sequence)
        """
        self.log(10 * "-" + f" Extracting Features with session window {session_reg} " + 10 * "-")
        
        self.extraction = SessionBasedExtraction(logging=self.logging)
        
        # Count events in each log sequence
        log_seq_df, Y = self.extraction.transform(x_data, self.event_col, session_reg, session_col, y_data)
        X_df = self._count_events_in_seq(log_seq_df, self.event_col)
        
        if y_data is not None: 
            assert X_df.shape[0] == len(Y), "Something went wrong, number of windows does not match the number of labels"
        
        # Store parameters for later use (in transform method)
        self.extraction_params = {"session_reg": session_reg, "session_col": session_col}
        self.events = X_df.columns.values.tolist()
            
        self._log_statistics(X_df, Y)
        
        return X_df, Y
    
    def fixed_windowing(self, x_data, wp, y_data = None):
        """ Split the log sequence into fixed windows based on time and date columns
        
        ### Args:
            x_data (DataFrame): raw log lines
            wp (WindowParams): parameters for windowing
            y_data (Numpy array): (optional) labels for the raw log lines
        
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids, each row represents a session
            Y (Array): array containing labels for each session (in order of appearance in the log sequence)
        """
        self.log(10 * "-" + f" Extracting Features with fixed window (size = {wp.window_size}m) " + 10 * "-")
        
        self.extraction = TimeBasedExtraction(self.logging)
        # window_step == window_size to create non-overlapping windows
        assert wp.window_step == 60 * wp.window_size, "Fixed windowing requires window size to be equal to window step"
        log_seq_df, Y = self.extraction.transform(x_data, self.event_col, wp, y_data)
        
        # Count events in each log sequence
        X_df = self._count_events_in_seq(log_seq_df, self.event_col)
        assert X_df.shape[0] == len(Y), "Something went wrong, number of windows does not match the number of labels"

        # Store parameters for later use (in transform method)
        self.extraction_params = {"wp": wp}
        self.events = X_df.columns.values.tolist()
            
        self._log_statistics(X_df, Y)
            
        return X_df, Y
    
    def sliding_windowing(self, x_data, wp, y_data = None):
        """ Split the log sequence into sliding windows based on time and date columns
        
        ### Args:
            x_data (DataFrame): raw log lines
            wp (WindowParams): parameters for windowing
            y_data (Numpy array): (optional) labels for the raw log lines
            
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids, each row represents a session
            Y (Array): array containing labels for each session (in order of appearance in the log sequence)
        """
        self.log(10 * "-" + f" Extracting Features with sliding window (size = {wp.window_size}m, step = {wp.window_step}s) " + 10 * "-")
        
        self.extraction = TimeBasedExtraction(self.logging)
        log_seq_df, Y = self.extraction.transform(x_data, self.event_col, wp, y_data)
        
        # Count events in each log sequence
        X_df = self._count_events_in_seq(log_seq_df, self.event_col)
        assert X_df.shape[0] == len(Y), "Something went wrong, number of windows does not match the number of labels"

        # Store parameters for later use (in transform method)
        self.extraction_params = {"wp": wp}
        self.events = X_df.columns.values.tolist()
            
        self._log_statistics(X_df, Y)
            
        return X_df, Y
    
    def transform(self, x_data, y_data = None):
        """ Transform the given log sequence into feature matrix with trained parameters
        This is usefull for transforming testing data to match the configuration of 
        feature extraction for training data (e.g term weighting, contrast based weighting, column names ...)
        
        ### Args:
            X_df (DataFrame): log sequence
            Y (Array): (optional) labels for the log sequence
            
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids
            Y (Array): array containing labels for each session (in order of appearance in the log sequence)
        """
        self.log(10 * "-" + " Transforming validation data " + 10 * "-")
        
        log_seq_df, Y = self.extraction.transform(x_data=x_data, y_data=y_data, event_col=self.event_col, **self.extraction_params)
        
        # Store session IDs
        if "SessionId" in log_seq_df.columns:
            self.session_ids = log_seq_df["SessionId"].values.tolist()
        
        # Count events in each log sequence
        X_df = self._count_events_in_seq(log_seq_df, self.event_col)
        
        # Fill missing events
        empty_events = set(self.events) - set(X_df.columns)
        for event in empty_events:
            X_df[event] = [0] * len(X_df)
        
        self._log_statistics(X_df, Y)
        
        return X_df, Y
    
    def split_data(self, X, Y = None, train_ratio = 0.5, split_type = "sequential"):
        """ Split the data into training and validation sets based on the specified split type and ratio

        ### Args:
            x_data (DataFrame): structured log data
            y_data (DataFame): labels for the structured log data (if available)
            train_ratio (float): ratio of the training set vs validation set (default = 0.5)
            split_type (str): type of split, 'uniform' or 'sequential' (default = 'sequential')
        
        ### Split types:
            uniform: Split the data with labels provided into training and validation sets so that the ratio of 
                    positive and negative samples is the same. When using this split type, the labels must be provided
            sequential: Split the data and into training and testing sets sequentially without regard to the ratio of 
                        positive and negative samples. When using this split type, the labels are optional

        ### Returns:
            (x_train, y_train), (x_test, y_test): the training and validation sets with optional labels
            
        ### References:
            Original code can be found at https://github.com/logpai/loglizer/blob/master/loglizer/dataloader.py
            Authors: LogPAI Team
        """
        
        assert split_type in ["uniform", "sequential"], "Invalid split type"
        self.log("Splitting the data into training and validation sets")
        
        if split_type == "uniform":
            assert Y is not None, "Labels must be provided when using uniform split"
            
            # Split the data into positive (anomaly) and negative (normal) samples
            pos_idx = Y > 0
            x_pos = X[pos_idx]
            y_pos = Y[pos_idx]
            x_neg = X[~pos_idx]
            y_neg = Y[~pos_idx]
            
            # Get the number of positive and negative samples to be included in the training set
            # based on the train_ratio
            train_pos = int(train_ratio * x_pos.shape[0])
            train_neg = int(train_ratio * x_neg.shape[0])
            
            # Concatenate the positive and negative samples based on the train_ratio
            x_train = pd.concat([x_pos[0:train_pos], x_neg[0:train_neg]])
            x_test = pd.concat([x_pos[train_pos:], x_neg[train_neg:]])
            y_train = np.concatenate([y_pos[0:train_pos], y_neg[0:train_neg]])
            y_test = np.concatenate([y_pos[train_pos:], y_neg[train_neg:]])
        elif split_type == "sequential":
            # Split the data into training and validation sets based on the train_ratio
            num_train = int(train_ratio * X.shape[0])
           
            x_train = X[0:num_train]
            x_test = X[num_train:]
            y_train = None
            y_test = None

            # If labels are provided, split them as well
            if Y is not None:
                y_train = Y[0:num_train]
                y_test = Y[num_train:]
                
        self.log(f"Total: {X.shape[0]}, Training: {x_train.shape[0]}, Validation: {x_test.shape[0]}")
        return (x_train, y_train), (x_test, y_test)
    
    def apply_weighting(self, X_df, tf_idf = True, contrast_w = False):
        """ Apply term weighting to the given log sequence 
        
        ### Args:
            X_df (DataFrame): log sequence
            tf_idf (bool): apply tf-idf based weighting
            contrast_w (bool): apply contrast based weighting
            
        ### Returns:
            X_df (DataFrame): weighted log sequence
        """
        if tf_idf:
            X_df = self._term_weighting(X_df)
            
        if contrast_w and self.events is not None:
            X_df = self._contrast_based_weighting(X_df)
        
        return X_df
    
    def _term_weighting(self, X_df):
        """ Apply tf-idf based weighting based to the given log sequence """
        self.log(f"using term weighting tf-idf")
        
        N = X_df.shape[0] # Overall number of sequences
        nt = X_df.astype(bool).sum(axis=0) + 1e-8 # Number of sequences containing the term t
        
        idf_vec = (N / nt).apply(lambda x: np.log(x))
        idf_vec = np.nan_to_num(idf_vec)
        idf_df = pd.DataFrame(np.tile(idf_vec, (N, 1)) * X_df, columns=X_df.columns)
        
        self.tf_idf = True
        
        return idf_df
    
    def _contrast_based_weighting(self, X_df):
        """ Apply contrast based weighting to the given log sequence """
        self.log(f"using contrast based weighting")
        
        # Calculate if event occurs in knowledge base
        contrast_vec = list(map(lambda x: 0.5 * int(x not in self.events), X_df.columns))
        
        # Merge the contraghted dataframe
        X_df = X_df.apply(lambda x: 0.5 * expit(x) + (x > 0) * contrast_vec, axis=1)
        
        self.contrast_w = True
        
        return X_df

    def _count_events_in_seq(self, data_df, event_col):
        """ Count the number of events in given log sequence 
        
        ### Args:
            data_df (DataFrame): log sequence
            event_col (str): name of the column containing the event id
        
        ### Returns
            X_df (DataFrame): count matrix with column names as event ids
        """
        # Create list of ordered dictionaries, each containing the count of each event in a sequence
        # Source: https://github.com/logpai/loglizer/blob/master/loglizer/preprocessing.py in fit_transform()
        X_counts = []
        for i in range(data_df.shape[0]):
            event_counts = Counter(data_df.loc[i, event_col])
            X_counts.append(event_counts)
            
        # Put the ordered dictionaries into a dataframe and fill in any missing events with 0
        X_df = pd.DataFrame(X_counts)
        X_df = X_df.fillna(0)  
        
        # Sort the columns by event id
        X_df = X_df.reindex(sorted(X_df.columns, key=lambda x: int(x[1:])), axis=1)
        return X_df
    
    def _log_statistics(self, X_df, Y):
        """ Calculate statistics about the log sequence """
        all_seq = X_df.shape[0]
        if Y is not None:
            norm_seq = np.argwhere(Y == 0).shape[0]
            anom_seq = np.argwhere(Y == 1).shape[0]
            self.log(f"Overall: {all_seq} sequences, Normal: {norm_seq}, Anomalies: {anom_seq}")
        else:
            self.log(f"Overall: {all_seq} sequences")