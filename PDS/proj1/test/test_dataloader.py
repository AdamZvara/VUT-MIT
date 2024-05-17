"""
Tests for DataLoader

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import unittest

import sys
sys.path.append("..") # Adds higher directory to python modules path

from src.DataLoader import DataLoader
from src.FeatureExtraction import FeatureExtraction

import os
base_path = os.path.dirname(os.path.abspath(__file__))

log_file = os.path.join(base_path, "dummy_data", "log_structured.csv")
label_file = os.path.join(base_path, "dummy_data", "labels_structured.csv")
label_file2 = os.path.join(base_path, "dummy_data", "labels_structured_nums.csv")

class DataLoaderTest(unittest.TestCase):
    """ Tests for DataLoader class """
    
    def setUp(self):
        # Since loading and splitting data are 2 different opertions and splitting can be only done
        # after FeatureExtraction, default session windowing is used here
        self.default_dl = DataLoader(logging=False)
        X, Y = self.default_dl.load_csv(log_file, label_file)
        self.fe = FeatureExtraction(event_col='EventId', logging=False)
        self.X, self.Y = self.fe.session_windowing(x_data=X, y_data=Y, session_reg=r'(blk_-?\d+)', session_col='Content')
        # Swap second row with the last one to see if sequential and uniform split differ
        self.X.iloc[1], self.X.iloc[-1] = self.X.iloc[-1].copy(), self.X.iloc[1].copy()
        self.Y[1], self.Y[-1] = self.Y[-1].copy(), self.Y[1].copy()
        
    def _event_num_to_str(self, events):
        return ["E" + str(i) for i in events]
    
    def test_load_csv_text_labels(self):
        dl = DataLoader(logging=False)
        X, Y = dl.load_csv(log_file, label_file)
        
        # Check if the data was loaded correctly
        self.assertIsNotNone(X)
        self.assertIsNotNone(Y)
        self.assertEqual(X.shape[0], 36)
        self.assertEqual(X.shape[0], len(Y))
        
        # Try loading binary labels
        _, Y2 = dl.load_csv(log_file, label_file2)
        self.assertListEqual(Y.tolist(), Y2.tolist())
        
    def test_seqsplit_half_success(self):
        (x_train, y_train), (x_test, y_test) = self.fe.split_data(self.X, self.Y)
        
        # Check if the data was split correctly
        self.assertAlmostEqual(x_train.shape[0], x_test.shape[0], delta=1)
        self.assertAlmostEqual(y_train.shape[0], y_train.shape[0], delta=1)
        self.assertEqual(y_train.tolist(), [0, 0])
        self.assertEqual(y_test.tolist(), [0, 1, 1])
        
    def test_seqsplit_ninety_success(self):
        (x_train, y_train), (x_test, y_test) = self.fe.split_data(X=self.X, Y=self.Y, train_ratio=0.9)
        
        # Check if the data was split correctly
        self.assertEqual(x_train.shape[0], 4)
        self.assertEqual(x_test.shape[0],  1)        
        self.assertEqual(y_train.tolist(), [0, 0, 0, 1])
        self.assertEqual(y_test.tolist(), [1])
        
    def test_unisplit_half_success(self):
        (x_train, y_train), (x_test, y_test) = self.fe.split_data(X=self.X, Y=self.Y, split_type='uniform')
        
        # Check if the data was split correctly
        self.assertAlmostEqual(x_train.shape[0], x_test.shape[0], delta=1)
        self.assertAlmostEqual(y_train.shape[0], y_train.shape[0], delta=1)
        self.assertEqual(y_train.tolist(), [1, 0])
        self.assertEqual(y_test.tolist(), [1, 0, 0])
        
    def test_unisplit_fifty_success(self):
        (x_train, y_train), (x_test, y_test) = self.fe.split_data(X=self.X, Y=self.Y, split_type='uniform')
        
        # Check if the data was split correctly
        self.assertAlmostEqual(x_train.shape[0], x_test.shape[0], delta=1)
        self.assertAlmostEqual(y_train.shape[0], y_train.shape[0], delta=1)
        self.assertEqual(y_train.tolist(), [1, 0])
        self.assertEqual(y_test.tolist(), [1, 0, 0])
        
    def test_nolabel_uniformsplit_fail(self):
        # Uniform split without labels should fail
        with self.assertRaises(AssertionError):
            self.fe.split_data(X=self.X, Y=None, split_type='uniform')
        
if __name__ == '__main__':
    unittest.main()