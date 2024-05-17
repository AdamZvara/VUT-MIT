"""
API for Log Clustering as described in "Log Clustering based Problem Identification for Online 
Service Systems"

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import numpy as np
import pandas as pd
import pickle as pkl
import matplotlib.pyplot as plt

from scipy.spatial.distance import pdist, squareform
from scipy.cluster.hierarchy import linkage, fcluster
from sklearn.metrics import accuracy_score, precision_recall_fscore_support

from .utils import Log

class LogCluster(Log):
    _cluster_col = "ClusterId"
    _noise = 1e-8
    
    def __init__(self, max_dist: int = None, threshold: int = None, contrast_w = False, logging: bool = True):
        super().__init__(self.__class__.__name__, logging)
        self.max_dist = max_dist
        self.threshold = threshold
        self.contrast_w = contrast_w
        
        # Knowledge base
        self.centroids = []
        self.events = None
    
    def fit(self, X):
        """ Fit LogCluster model on the given data X 
        
        ### Args:
            X (pd.DataFrame): Data to fit the model on
        """
        self.log(10 * "-" + f" Fitting LogCluster model " + 10 * "-")
        
        # Add small noise to the data to avoid zero-length vectors in cosine distance
        X += self._noise
        
        # Agglomerative clustering
        p_dist = pdist(X, metric='cosine')
        Z = linkage(p_dist, 'complete')
        cluster_index = fcluster(Z, self.max_dist, criterion='distance')
        
        # Extract representatives and events
        X.insert(0, self._cluster_col, cluster_index)
        self._init_knowledge_base(X, p_dist)
        
        self.log(f"Number of clusters: {len(set(cluster_index))}")
        
    def predict(self, X):
        """ Predict anomalies in the given data X

        ### Args:
            X (pd.DataFrame): Data to predict on

        ### Returns:
            y_pred (np.ndarray): Predicted labels
            distcs (np.ndarray): Distances to nearest cluster
        """
        y_pred = np.zeros(X.shape[0])
        
        self._synchronize_events(X)
        
        distcs = []
        # Calculate distance of each prediction to nearest cluster
        for i in range(X.shape[0]):
            # Add current sample to centroids
            centroids = self.centroids.copy()
            centroids.insert(0, X.iloc[i].to_list())
            # Calculate cosine distance to all centroids
            dist = squareform(pdist(centroids, metric='cosine'))[0]
            min_dist = np.min(dist[1:])
            distcs.append(min_dist)
            if min_dist > self.threshold:
                y_pred[i] = 1
        return y_pred, np.array(distcs)
    
    def evaluate(self, X, y_true, debug = False):
        """ Evaluate model on the given data X and true labels y_true

        ### Args:
            X (pd.DataFrame): Data to evaluate on
            y_true (np.ndarray): True labels
            debug (bool, optional): Print debug information

        ### Returns:
            precision (float): Precision score
            recall (float): Recall score
            f1 (float): F1-measure score
            
        Source: https://github.com/logpai/loglizer/blob/master/loglizer/models/LogClustering.py in evaluate()
        """
        y_pred, distances = self.predict(X)
        
        if debug:
            anomal = np.histogram(distances[y_true == 1], bins=30)
            for i in range(len(anomal[0])):
                self.log(f"Anomalies in bin {anomal[1][i]}: {anomal[0][i]}")
                
            normal = np.histogram(distances[y_true == 0], bins=30)
            for i in range(len(normal[0])):
                self.log(f"Normals in bin {normal[1][i]}: {normal[0][i]}")
            
        self.log("Accuracy: {:.3f}".format(accuracy_score(y_true, y_pred)))    
        precision, recall, f1, _ = precision_recall_fscore_support(y_true, y_pred, average='binary')
        self.log('Precision: {:.3f}, recall: {:.3f}, F1-measure: {:.3f}\n'.format(precision, recall, f1))
        return precision, recall, f1
    
    def export_base(self, path, feature_extraction):
        """ Export knowledge base to file

        ### Args:
            path (str): Path to export knowledge base to
            fe (FeatureExtraction): Feature extraction object (to transform validation data)
        """
        self.log(f"Exporting knowledge base")
        storage = {"centroids": self.centroids, "events": self.events, "dist": self.max_dist, "thr" : self.threshold, "contrast_w": self.contrast_w, "feature_extraction": feature_extraction}
        file = open(path, "wb")
        pkl.dump(storage, file)
        file.close()
        
    def import_base(self, path):
        """ Import knowledge base from file

        ### Args:
            path (str): Path to import knowledge base from
        """
        self.log(f"Importing knowledge base")
        file = open(path, "rb")
        storage = pkl.load(file)
        file.close()
        
        self.centroids = storage["centroids"]
        self.events = storage["events"]
        self.max_dist = storage["dist"]
        self.threshold = storage["thr"]
        self.contrast_w = storage["contrast_w"]
        
        return storage["feature_extraction"]
    
    def retrieve_anomalies(self, X):
        """ Retrieve anomalies from the given data X

        ### Args:
            X (pd.DataFrame): Data to retrieve anomalies from

        ### Returns:
            anomalies (pd.DataFrame): Anomalies from the given data
        """
        y_pred, _ = self.predict(X)
        anomalies = X[y_pred == 1]
        return anomalies
    
    def _init_knowledge_base(self, X, p_dist):
        """ Initialize knowledge base with centroids and events

        ### Args:
            X (pd.DataFrame): Data to initialize knowledge base on
            p_dist (np.ndarray): Pairwise distances between samples
        """
        # Store events
        self.events = X.columns[1:]
        
        # Extract centroids
        distances = squareform(p_dist)
        for cluster in set(X[self._cluster_col]):
            # Get all events from current cluster
            cluster_idx = X[self._cluster_col] == cluster
            # Calculate scores for each event in cluster
            scores = np.divide(np.sum(distances[cluster_idx], axis=1), cluster_idx.shape[0])
            # Get event with lowest score as centroid
            centroid = X[cluster_idx].iloc[np.argmin(scores)][1:]
            self.centroids.append(centroid.to_list())
            
    def _synchronize_events(self, X):
        """ Synchronize events in the given data X with the knowledge base """
        # Get difference between current and stored events
        new_events = list(set(X.columns) - set(self.events))
        # Add new events to centroids
        new_centroids = pd.DataFrame(self.centroids, columns=self.events)
        if self.contrast_w:
            new_centroids[new_events] = 0.25 # Default value for contrast weighting
        else:
            new_centroids[new_events] = 0
        # Sort columns
        new_centroids = new_centroids.reindex(sorted(new_centroids.columns, key=lambda x: int(x[1:])), axis=1)
        self.events = new_centroids.columns
        self.centroids = new_centroids.values.tolist()