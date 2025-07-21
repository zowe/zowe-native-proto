"""
Simplified test suite for zDS (z/OS Data Set) Flask routes.
Tests core functionality for data set operations.
"""

import pytest
import json
import sys
import os
from unittest.mock import patch, MagicMock
from flask import Flask

# Add the parent directory to the Python path to import from routes/
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from routes import zds_bp


@pytest.fixture
def app():
    """Create a Flask test application."""
    app = Flask(__name__)
    app.register_blueprint(zds_bp)
    app.config['TESTING'] = True
    return app


@pytest.fixture
def client(app):
    """Create a test client for the Flask application."""
    return app.test_client()


@pytest.fixture
def mock_dataset_entry():
    """Create a mock dataset entry object."""
    entry = MagicMock()
    entry.name = "USER.TEST.DATASET"
    entry.dsorg = "PS"
    entry.volser = "VOL001"
    entry.recfm = "FB"
    entry.migr = False
    return entry


@pytest.fixture
def mock_member_entry():
    """Create a mock member entry object."""
    member = MagicMock()
    member.name = "MEMBER01"
    return member


class TestListDataSets:
    """Test cases for the list_data_sets endpoint."""
    
    @patch('routes.zds_routes.zds.list_data_sets')
    def test_list_data_sets_success(self, mock_list_data_sets, client, mock_dataset_entry):
        """Test successful listing of data sets."""
        mock_list_data_sets.return_value = [mock_dataset_entry]
        
        response = client.get('/zosmf/restfiles/ds?dslevel=USER.TEST')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 1
        assert data['items'][0]['name'] == 'USER.TEST.DATASET'
        mock_list_data_sets.assert_called_once_with('USER.TEST.**')
    
    @patch('routes.zds_routes.zds.list_data_sets')
    def test_list_data_sets_with_attributes(self, mock_list_data_sets, client, mock_dataset_entry):
        """Test listing data sets with detailed attributes."""
        mock_list_data_sets.return_value = [mock_dataset_entry]
        
        response = client.get('/zosmf/restfiles/ds?dslevel=USER.TEST&attributes=true')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['items'][0]['dsorg'] == 'PS'
        assert data['items'][0]['volser'] == 'VOL001'
    
    def test_list_data_sets_missing_dslevel(self, client):
        """Test listing data sets without dslevel parameter."""
        response = client.get('/zosmf/restfiles/ds')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'dslevel parameter is required' in data['error']
    
    @patch('routes.zds_routes.zds.list_data_sets')
    def test_list_data_sets_exception(self, mock_list_data_sets, client):
        """Test handling of exceptions during data set listing."""
        mock_list_data_sets.side_effect = Exception("System error")
        
        response = client.get('/zosmf/restfiles/ds?dslevel=USER.TEST')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not list data set' in data['error']


class TestListDataSetMembers:
    """Test cases for the list_data_set_members endpoint."""
    
    @patch('routes.zds_routes.zds.list_members')
    def test_list_members_success(self, mock_list_members, client, mock_member_entry):
        """Test successful listing of data set members."""
        mock_list_members.return_value = [mock_member_entry]
        
        response = client.get('/zosmf/restfiles/ds/USER.TEST.PDS/member')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 1
        assert data['datasetName'] == 'USER.TEST.PDS'
        assert data['items'][0]['name'] == 'MEMBER01'
        mock_list_members.assert_called_once_with('USER.TEST.PDS')
    
    @patch('routes.zds_routes.zds.list_members')
    def test_list_members_exception(self, mock_list_members, client):
        """Test handling of exceptions during member listing."""
        mock_list_members.side_effect = Exception("Data set not found")
        
        response = client.get('/zosmf/restfiles/ds/USER.TEST.PDS/member')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not read data set' in data['error']


class TestReadDataSetOrMember:
    """Test cases for the read_data_set_or_member endpoint."""
    
    @patch('routes.zds_routes.zds.read_data_set')
    def test_read_data_set_success(self, mock_read_data_set, client):
        """Test successful reading of a data set."""
        mock_read_data_set.return_value = "DATA SET CONTENT"
        
        response = client.get('/zosmf/restfiles/ds/USER.TEST.PS')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "DATA SET CONTENT"
        assert data['datasetName'] == 'USER.TEST.PS'
        assert data['format'] == 'text'
        mock_read_data_set.assert_called_once_with('USER.TEST.PS', '')
    
    @patch('routes.zds_routes.zds.read_data_set')
    def test_read_member_success(self, mock_read_data_set, client):
        """Test successful reading of a data set member."""
        mock_read_data_set.return_value = "MEMBER CONTENT"
        
        response = client.get('/zosmf/restfiles/ds/USER.TEST.PDS(MEMBER01)')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "MEMBER CONTENT"
        assert data['datasetName'] == 'USER.TEST.PDS'
        assert data['memberName'] == 'MEMBER01'
        mock_read_data_set.assert_called_once_with('USER.TEST.PDS(MEMBER01)', '')
    
    @patch('routes.zds_routes.zds.read_data_set')
    def test_read_data_set_exception(self, mock_read_data_set, client):
        """Test handling of exceptions during data set reading."""
        mock_read_data_set.side_effect = Exception("Data set not found")
        
        response = client.get('/zosmf/restfiles/ds/USER.TEST.PS')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not read data set' in data['error']


class TestWriteDataSetOrMember:
    """Test cases for the write_data_set_or_member endpoint."""
    
    @patch('routes.zds_routes.zds.write_data_set')
    def test_write_data_set_success(self, mock_write_data_set, client):
        """Test writing to data set with JSON records."""
        mock_write_data_set.return_value = "etag123"
        
        response = client.put('/zosmf/restfiles/ds/USER.TEST.PS',
                            json={"records": "NEW CONTENT"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.PS'
        assert data['etag'] == "etag123"
        mock_write_data_set.assert_called_once_with('USER.TEST.PS', 'NEW CONTENT', '', '')
    
    @patch('routes.zds_routes.zds.write_data_set')
    def test_write_member_success(self, mock_write_data_set, client):
        """Test writing to data set member."""
        mock_write_data_set.return_value = "etag456"
        
        response = client.put('/zosmf/restfiles/ds/USER.TEST.PDS(*MEMBER01*)',
                            json={"records": "MEMBER CONTENT"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.PDS'
        assert data['memberName'] == 'MEMBER01'
        mock_write_data_set.assert_called_once_with('USER.TEST.PDS(MEMBER01)', 'MEMBER CONTENT', '', '')
    
    def test_write_data_set_no_data(self, client):
        """Test writing to data set with no data."""
        response = client.put('/zosmf/restfiles/ds/USER.TEST.PS',
                            json={},
                            content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'request body cannot be empty' in data['error']
    
    @patch('routes.zds_routes.zds.write_data_set')
    def test_write_data_set_exception(self, mock_write_data_set, client):
        """Test handling of exceptions during data set writing."""
        mock_write_data_set.side_effect = Exception("Write failed")
        
        response = client.put('/zosmf/restfiles/ds/USER.TEST.PS',
                            json={"records": "CONTENT"},
                            content_type='application/json')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not write to data set' in data['error']


class TestCreateMember:
    """Test cases for the create_member endpoint."""
    
    @patch('routes.zds_routes.zds.create_member')
    def test_create_member_success(self, mock_create_member, client):
        """Test successful creation of a data set member."""
        response = client.post('/zosmf/restfiles/ds/USER.TEST.PDS/NEWMEMBER')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.PDS'
        assert data['memberName'] == 'NEWMEMBER'
        mock_create_member.assert_called_once_with('USER.TEST.PDS(NEWMEMBER)')
    
    @patch('routes.zds_routes.zds.create_member')
    def test_create_member_exception(self, mock_create_member, client):
        """Test handling of exceptions during member creation."""
        mock_create_member.side_effect = Exception("Creation failed")
        
        response = client.post('/zosmf/restfiles/ds/USER.TEST.PDS/NEWMEMBER')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not create member' in data['error']


class TestCreateDataSet:
    """Test cases for the create_data_set endpoint."""
    
    @patch('routes.zds_routes.zds.create_data_set')
    @patch('routes.zds_routes.zds.DS_ATTRIBUTES')
    def test_create_data_set_success(self, mock_ds_attributes, mock_create_data_set, client):
        """Test successful creation of a data set."""
        mock_attributes = MagicMock()
        mock_ds_attributes.return_value = mock_attributes
        
        response = client.post('/zosmf/restfiles/ds/USER.TEST.NEWDS')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.NEWDS'
        mock_create_data_set.assert_called_once_with('USER.TEST.NEWDS', mock_attributes)
    
    @patch('routes.zds_routes.zds.create_data_set')
    @patch('routes.zds_routes.zds.DS_ATTRIBUTES')
    def test_create_data_set_with_attributes(self, mock_ds_attributes, mock_create_data_set, client):
        """Test creation of data set with specific attributes."""
        mock_attributes = MagicMock()
        mock_ds_attributes.return_value = mock_attributes
        
        response = client.post('/zosmf/restfiles/ds/USER.TEST.NEWDS?dsorg=PS&lrecl=80')
        
        assert response.status_code == 201
        mock_create_data_set.assert_called_once_with('USER.TEST.NEWDS', mock_attributes)
    
    @patch('routes.zds_routes.zds.create_data_set')
    @patch('routes.zds_routes.zds.DS_ATTRIBUTES')
    def test_create_data_set_exception(self, mock_ds_attributes, mock_create_data_set, client):
        """Test handling of exceptions during data set creation."""
        mock_attributes = MagicMock()
        mock_ds_attributes.return_value = mock_attributes
        mock_create_data_set.side_effect = Exception("Creation failed")
        
        response = client.post('/zosmf/restfiles/ds/USER.TEST.NEWDS')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not create data set' in data['error']


class TestDeleteDataSet:
    """Test cases for the delete_data_set endpoint."""
    
    @patch('routes.zds_routes.zds.delete_data_set')
    def test_delete_data_set_success(self, mock_delete_data_set, client):
        """Test successful deletion of a data set."""
        response = client.delete('/zosmf/restfiles/ds/USER.TEST.OLDDS')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.OLDDS'
        mock_delete_data_set.assert_called_once_with('USER.TEST.OLDDS')
    
    @patch('routes.zds_routes.zds.delete_data_set')
    def test_delete_member_success(self, mock_delete_data_set, client):
        """Test successful deletion of a data set member."""
        response = client.delete('/zosmf/restfiles/ds/USER.TEST.PDS(OLDMEMBER)')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['datasetName'] == 'USER.TEST.PDS'
        assert data['memberName'] == 'OLDMEMBER'
        mock_delete_data_set.assert_called_once_with('USER.TEST.PDS(OLDMEMBER)')
    
    @patch('routes.zds_routes.zds.delete_data_set')
    def test_delete_data_set_exception(self, mock_delete_data_set, client):
        """Test handling of exceptions during data set deletion."""
        mock_delete_data_set.side_effect = Exception("Deletion failed")
        
        response = client.delete('/zosmf/restfiles/ds/USER.TEST.OLDDS')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not delete data set' in data['error']


if __name__ == '__main__':
    pytest.main([__file__])