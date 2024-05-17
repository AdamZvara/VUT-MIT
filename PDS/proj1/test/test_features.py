"""
Tests for FeatureExtraction

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import unittest
from scipy.special import expit
from numpy import log

import sys
sys.path.append("..") # Adds higher directory to python modules path

from src.DataLoader import DataLoader
from src.FeatureExtraction import FeatureExtraction
from src.FeatureExtractionModels.TimeWindow import WindowParams

import os
base_path = os.path.dirname(os.path.abspath(__file__))

log_file = os.path.join(base_path, "dummy_data", "log_structured.csv")
label_file = os.path.join(base_path, "dummy_data", "labels_structured.csv")

class FeatureExtractorTest(unittest.TestCase):
    """ Tests for FeatureExtractor class """
    
    def setUp(self):
        self.dl = DataLoader(logging=False)
        self.X1, self.Y1 = self.dl.load_csv(log_file, label_file)
        
    def _session_feature_extract(self, tf_idf_weighting, contrast_weighting = False):
        self.fe = FeatureExtraction('EventId', False)
        X, Y = self.fe.session_windowing(self.X1, r'(blk_-?\d+)', 'Content', self.Y1)
        X = self.fe.apply_weighting(X, tf_idf_weighting, contrast_weighting)
        return self.fe.split_data(X, Y, 1)
        
    def _fixed_feature_extract(self, date, date_fmts, weighting=False):
        self.fe = FeatureExtraction('EventId', False)
        
        wparams = WindowParams(window_size=60, window_step=60 * 60, time_col="Time", time_fmt="%H%M%S", date_col=date, date_fmt=date_fmts)
        X, Y = self.fe.fixed_windowing(self.X1, wparams, self.Y1)
        X = self.fe.apply_weighting(X, weighting, False)
        
        return self.fe.split_data(X, Y, 1)
        
    def _sliding_feature_extract(self, date, date_fmts):
        self.fe = FeatureExtraction('EventId', False)
        
        wparams = WindowParams(window_size=60, window_step=60 * 30, time_col="Time", time_fmt="%H%M%S", date_col=date, date_fmt=date_fmts)
        X, Y = self.fe.sliding_windowing(self.X1, wparams, self.Y1)
        
        return self.fe.split_data(X, Y, 1)
        
    def test_session_no_weighting_success(self):
        (x_train, y_train), (_, _)  = self._session_feature_extract(tf_idf_weighting=False)
        
        # Test few sample from the training set
        self.assertIsNotNone(x_train)
        self.assertIsNotNone(y_train)
        
        # Manually calculated event counts for the first five rows of the training set
        counts = [[1, 2, 1, 2, 2, 1, 2],
                  [3, 1, 0, 1, 0, 0, 0], 
                  [1, 2, 2, 0, 0, 0, 0], 
                  [0, 1, 1, 0, 0, 0, 2], 
                  [0, 0, 5, 6, 0, 0, 0]]
        
        # Test the first five rows of the training set with manually calculated values
        x_train.iloc[0:5].apply(lambda row: self.assertListEqual(row.to_list(), counts.pop(0)), axis=1)
        
        # Test the labels
        self.assertListEqual(y_train[:5].tolist(), [0, 1, 0, 1, 0])
    
    def test_session_weighting_success(self):
        (x_train, _), (_, _)  = self._session_feature_extract(tf_idf_weighting=True)
        
        # Test the first row of the training set with manually calculated values
        exp_x = [0.510826, 0.446287, 0.223144, 1.021651, 3.218876, 1.609438, 1.832581]
        for first, second in zip(x_train.iloc[0].to_list(), exp_x):
            self.assertAlmostEqual(first, second, delta=0.0001)
            
    def test_session_contrast_weighting_success(self):
        (x, _), (_, _)  = self._session_feature_extract(tf_idf_weighting=True, contrast_weighting=True)
        
        # Remove some columns in knowledge base to simulate contrast weighting
        self.fe.events = self.fe.events[2:]
        (x_train, _) = self.fe.transform(self.X1, self.Y1)
        x_train = self.fe.apply_weighting(X_df=x_train, tf_idf=True, contrast_w=True)
        
        # Test the first row of the training set with manually calculated values
        exp_x = [0.510826, 0.446287, 0.223144, 1.021651, 3.218876, 1.609438, 1.832581]
        exp_x = list(map(lambda x: 0.5 * expit(x), exp_x))
        exp_x[0] += 0.5
        exp_x[1] += 0.5

        for first, second in zip(x_train.iloc[0].to_list(), exp_x):
            self.assertAlmostEqual(first, second, delta=0.0001)
    
    def test_session_different_labels_fail(self):
        self.Y1[0] = 1 # First sessionId has different label than the rest
        with self.assertRaises(AssertionError):
            self._session_feature_extract(tf_idf_weighting=False)
            
    def test_fixed_nodate_success(self):
        (x, y), (_, _) = self._fixed_feature_extract(date=None, date_fmts=None)
        
        exp_cnt = [[3, 0, 0, 2, 0, 1, 0], [0, 0, 4, 2, 0, 0, 3], [1, 0, 4, 4, 2, 0, 1], [1, 6, 1, 1, 0, 0, 0]]
        
        # # Test training set with manually calculated values
        x.iloc[:].apply(lambda row: self.assertListEqual(row.to_list(), exp_cnt.pop(0)), axis=1)
        
        # # Test the labels
        self.assertListEqual(y.tolist(), 4 * [1])
        
    def test_fixed_nodate_weighted_success(self):
        # Data is split into 1hr non-overlapping blocks
        (x, y), (_, _) = self._fixed_feature_extract(date=None, date_fmts=None, weighting=True)
        
        exp_cnt = [[3, 0, 0, 2, 0, 1, 0], [0, 0, 4, 2, 0, 0, 3], [1, 0, 4, 4, 2, 0, 1], [1, 6, 1, 1, 0, 0, 0]]
        tmp = [3, 1, 3, 4, 1, 1, 2]
        exp_weighted = exp_cnt.copy()
        for i in range(4):
            for j in range(7):
                exp_weighted[i][j] = log(4 / tmp[j]) * exp_cnt[i][j]
        
        # Test training set with calculated values
        for line in x.iterrows():
            for first, second in zip(line[1].to_list(), exp_weighted.pop(0)):
                self.assertAlmostEqual(first, second, delta=0.0001)
        
    def test_fixed_date_single_col_success(self):
        # Change the date and time for couple events
        self.X1.loc[self.X1["Id"] > 10, "Date"] = 91109
        self.X1.loc[self.X1["Id"] > 10, "Time"] = 190000
        
        (x, y), (_, _) = self._fixed_feature_extract(date=["Date"], date_fmts="%d%m%y", weighting=False)
        exp_cnt = [[3, 0, 0, 2, 0, 1, 0], [0, 0, 3, 1, 0, 0, 0], [2, 6, 6, 6, 2, 0, 4]]
        
        # # Test training set with manually calculated values
        x.iloc[:].apply(lambda row: self.assertListEqual(row.to_list(), exp_cnt.pop(0)), axis=1)
        
        # # Test the labels
        self.assertListEqual(y.tolist(), [1, 0, 1])
        
    def test_fixed_date_multiple_cols_success(self):
        # Change the date and time for couple events
        self.X1["Month"] = "Jan"
        self.X1["Year"] = "2022"
        self.X1["Day"] = "01"
        
        (x, y), (_, _) = self._fixed_feature_extract(date=["Year", "Month", "Day"], date_fmts="%Y%b%d", weighting=False)
        
        exp_cnt = [[3, 0, 0, 2, 0, 1, 0], [0, 0, 4, 2, 0, 0, 3], [1, 0, 4, 4, 2, 0, 1], [1, 6, 1, 1, 0, 0, 0]]
        
        # Test training set with manually calculated values
        x.iloc[:].apply(lambda row: self.assertListEqual(row.to_list(), exp_cnt.pop(0)), axis=1)
        
        # Test the labels
        self.assertListEqual(y.tolist(), 4 * [1])
                
    def test_sliding_nodate_success(self):
        (x, y), (_, _) = self._sliding_feature_extract(date=None, date_fmts=None)
        
        exp_cnt = [[3, 0, 0, 2, 0, 1, 0], [0, 0, 3, 2, 0, 0, 2], [0, 0, 4, 2, 0, 0, 3], [1, 0, 4, 5, 1, 0, 2],
                   [1, 0, 4, 4, 2, 0, 1], [1, 0, 2, 1, 1, 0, 0], [1, 6, 1, 1, 0, 0, 0], [0, 6, 0, 0, 0, 0, 0]]
        
        # Test training set with manually calculated values
        x.iloc[:].apply(lambda row: self.assertListEqual(row.to_list(), exp_cnt.pop(0)), axis=1)
        
        # Test the labels
        self.assertListEqual(y.tolist(), 8 * [1])        