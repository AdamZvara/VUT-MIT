"""
Tests for clustering

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 4/2024
"""

import unittest

from src.DataLoader import DataLoader
from src.FeatureExtraction import FeatureExtraction
from src.LogCluster import LogCluster

import os
base_path = os.path.dirname(os.path.abspath(__file__))

log_file = os.path.join(base_path, "dummy_data", "log_structured.csv")
label_file = os.path.join(base_path, "dummy_data", "labels_structured.csv")

class ClusteringTest(unittest.TestCase):
    def setUp(self):
        x_raw, y_raw = DataLoader(False).load_csv(log_file, label_file)
        
        self.fe = FeatureExtraction(event_col='EventId', logging=False)
        x, self.y = self.fe.session_windowing(x_raw, '(blk_-?\\d+)', 'Content', y_raw)
        self.x = self.fe.apply_weighting(x, tf_idf=True, contrast_w=False)
    
    def test_create_model_success(self):
        test = self.x.copy()
        
        # Just a simple test to check, if the model can be created
        model = LogCluster(0.3, 0.3, False, False)
        model.fit(self.x)
        
        self.assertIsNotNone(model)
        self.assertEqual(len(model.centroids), 5)
        
        anom = model.predict(test)
        self.assertIsNotNone(anom)
        
    def test_model_export_import_base(self):
        model = LogCluster(0.3, 0.3, False, False)
        model.fit(self.x)
        
        # Export knowledge base
        model.export_base('test_model', self.fe)
        self.assertTrue(os.path.exists('test_model'))
        
        # Import knowledge base
        model2 = LogCluster(0.3, 0.3, False, False)
        model2.import_base('test_model')
        self.assertListEqual(model.centroids, model2.centroids)
        
        # Cleanup 
        os.remove('test_model')