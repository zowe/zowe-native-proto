"""
Test suite for zUSF (z/OS Unix System Files) Flask routes.
Tests all endpoints for USS file listing, reading, writing, creating, deleting, and utility operations.
"""

import pytest
import sys
import os
import json
from unittest.mock import patch
from flask import Flask

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from routes import zusf_bp

@pytest.fixture
def app():
    """Create a Flask test application."""
    app = Flask(__name__)
    app.register_blueprint(zusf_bp)
    app.config['TESTING'] = True
    return app


@pytest.fixture
def client(app):
    """Create a test client for the Flask application."""
    return app.test_client()


class TestListUSSFiles:
    """Test cases for the list_uss_files endpoint."""
    
    @patch('zusf_routes.zusf.list_uss_dir')
    def test_list_uss_files_success_json(self, mock_list_uss_dir, client):
        """Test successful listing of USS files with JSON response."""
        mock_list_uss_dir.return_value = '[{"name": "file1.txt", "type": "file"}, {"name": "dir1", "type": "directory"}]'
        
        response = client.get('/zosmf/restfiles/fs?path=/tmp')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['path'] == '/tmp'
        assert data['returnedRows'] == 2
        assert len(data['items']) == 2
        assert data['items'][0]['name'] == 'file1.txt'
        assert data['format'] == 'short'
        assert data['showHidden'] == False
        mock_list_uss_dir.assert_called_once_with('/tmp')
    
    @patch('zusf_routes.zusf.list_uss_dir')
    def test_list_uss_files_success_non_json(self, mock_list_uss_dir, client):
        """Test successful listing with non-JSON response."""
        mock_list_uss_dir.return_value = 'file1.txt\ndir1\n'
        
        response = client.get('/zosmf/restfiles/fs?path=/home')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 2
        assert data['items'][0]['name'] == 'file1.txt'
        assert data['items'][1]['name'] == 'dir1'
    
    def test_list_uss_files_missing_path(self, client):
        """Test listing files without path parameter."""
        response = client.get('/zosmf/restfiles/fs')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'path parameter is required' in data['error']
    
    @patch('zusf_routes.zusf.list_uss_dir')
    def test_list_uss_files_with_options(self, mock_list_uss_dir, client):
        """Test listing files with all and long options."""
        mock_list_uss_dir.return_value = '[]'
        
        response = client.get('/zosmf/restfiles/fs?path=/tmp&all=true&long=true')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['format'] == 'long'
        assert data['showHidden'] == True
        assert len(data['warnings']) == 2
        assert 'all parameter provided but not supported' in data['warnings'][0]
        assert 'long parameter provided but not supported' in data['warnings'][1]
    
    @patch('zusf_routes.zusf.list_uss_dir')
    def test_list_uss_files_exception(self, mock_list_uss_dir, client):
        """Test handling of exceptions during file listing."""
        mock_list_uss_dir.side_effect = Exception("Access denied")
        
        response = client.get('/zosmf/restfiles/fs?path=/restricted')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not list USS files' in data['error']
        assert 'Access denied' in data['details']


class TestReadUSSFile:
    """Test cases for the read_uss_file endpoint."""
    
    @patch('zusf_routes.zusf.read_uss_file')
    def test_read_uss_file_success(self, mock_read_uss_file, client):
        """Test successful reading of a USS file."""
        mock_read_uss_file.return_value = "Hello, World!"
        
        response = client.get('/zosmf/restfiles/fs/tmp/test.txt')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "Hello, World!"
        assert data['filePath'] == '/tmp/test.txt'
        assert data['format'] == 'text'
        mock_read_uss_file.assert_called_once_with('/tmp/test.txt', '')
    
    @patch('zusf_routes.zusf.read_uss_file')
    def test_read_uss_file_with_encoding(self, mock_read_uss_file, client):
        """Test reading file with specific encoding."""
        mock_read_uss_file.return_value = "Test content"
        
        response = client.get('/zosmf/restfiles/fs/tmp/test.txt?encoding=UTF-8')
        
        assert response.status_code == 200
        mock_read_uss_file.assert_called_once_with('/tmp/test.txt', 'UTF-8')
    
    @patch('zusf_routes.zusf.read_uss_file')
    @patch('zusf_routes.time.time')
    def test_read_uss_file_with_etag(self, mock_time, mock_read_uss_file, client):
        """Test reading file with ETag generation."""
        mock_read_uss_file.return_value = "Test content"
        mock_time.return_value = 1234567890
        
        response = client.get('/zosmf/restfiles/fs/tmp/test.txt?return-etag=true')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert 'etag' in data
        assert data['etag'] is not None
    
    @patch('zusf_routes.zusf.read_uss_file')
    def test_read_uss_file_bytes_format(self, mock_read_uss_file, client):
        """Test reading file in bytes format."""
        mock_read_uss_file.return_value = "Hi"
        
        response = client.get('/zosmf/restfiles/fs/tmp/test.txt?response-format-bytes=true')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['format'] == 'bytes'
        assert data['records'] == [72, 105]  # ASCII values for 'H' and 'i'
    
    @patch('zusf_routes.zusf.read_uss_file')
    def test_read_uss_file_exception(self, mock_read_uss_file, client):
        """Test handling of exceptions during file reading."""
        mock_read_uss_file.side_effect = Exception("File not found")
        
        response = client.get('/zosmf/restfiles/fs/tmp/nonexistent.txt')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not view USS file' in data['error']
        assert 'File not found' in data['details']


class TestWriteUSSFile:
    """Test cases for the write_uss_file endpoint."""
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_json_records(self, mock_write_uss_file, client):
        """Test writing to USS file with JSON records."""
        mock_write_uss_file.return_value = "etag123"
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt',
                            json={"records": "Hello, World!"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['success'] == True
        assert data['etag'] == "etag123"
        assert data['filePath'] == '/tmp/test.txt'
        mock_write_uss_file.assert_called_once_with('/tmp/test.txt', 'Hello, World!', '', '')
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_bytes_format(self, mock_write_uss_file, client):
        """Test writing to USS file with bytes format."""
        mock_write_uss_file.return_value = "etag456"
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt',
                            json={"records": [72, 105]},  # 'Hi' in ASCII
                            content_type='application/json')
        
        assert response.status_code == 200
        mock_write_uss_file.assert_called_once_with('/tmp/test.txt', 'Hi', '', '')
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_raw_data(self, mock_write_uss_file, client):
        """Test writing to USS file with raw data."""
        mock_write_uss_file.return_value = "etag789"
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt',
                            data="Raw text data",
                            content_type='text/plain')
        
        assert response.status_code == 200
        mock_write_uss_file.assert_called_once_with('/tmp/test.txt', 'Raw text data', '', '')
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_with_encoding_and_etag(self, mock_write_uss_file, client):
        """Test writing file with encoding and ETag."""
        mock_write_uss_file.return_value = "newtag"
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt?encoding=UTF-8&etag=oldtag',
                            json={"records": "Content"},
                            content_type='application/json')
        
        assert response.status_code == 200
        mock_write_uss_file.assert_called_once_with('/tmp/test.txt', 'Content', 'UTF-8', 'oldtag')
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_etag_only(self, mock_write_uss_file, client):
        """Test writing file with etag-only response."""
        mock_write_uss_file.return_value = "etag123"
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt?etag-only=true',
                            json={"records": "Content"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['etag'] == "etag123"
        assert data['created'] == True
        assert 'message' not in data
    
    def test_write_uss_file_no_data(self, client):
        """Test writing to USS file with no data."""
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt',
                            json={},
                            content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'no data provided to write' in data['error']
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_write_uss_file_exception(self, mock_write_uss_file, client):
        """Test handling of exceptions during file writing."""
        mock_write_uss_file.side_effect = Exception("Permission denied")
        
        response = client.put('/zosmf/restfiles/fs/tmp/test.txt',
                            json={"records": "Content"},
                            content_type='application/json')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not write to USS file' in data['error']
        assert 'Permission denied' in data['details']


class TestCreateUSSFileOrDir:
    """Test cases for the create_uss_file_or_dir endpoint."""
    
    @patch('zusf_routes.zusf.create_uss_file')
    def test_create_uss_file_success(self, mock_create_uss_file, client):
        """Test successful creation of a USS file."""
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             json={"type": "file"},
                             content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['success'] == True
        assert data['path'] == '/tmp/newfile.txt'
        assert data['type'] == 'file'
        assert data['mode'] == '644'
        mock_create_uss_file.assert_called_once_with('/tmp/newfile.txt', '644')
    
    @patch('zusf_routes.zusf.create_uss_dir')
    def test_create_uss_directory_success(self, mock_create_uss_dir, client):
        """Test successful creation of a USS directory."""
        response = client.post('/zosmf/restfiles/fs/tmp/newdir',
                             json={"type": "directory"},
                             content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['success'] == True
        assert data['path'] == '/tmp/newdir'
        assert data['type'] == 'directory'
        assert data['mode'] == '755'
        mock_create_uss_dir.assert_called_once_with('/tmp/newdir', '755')
    
    @patch('zusf_routes.zusf.create_uss_file')
    def test_create_uss_file_with_symbolic_mode(self, mock_create_uss_file, client):
        """Test creating file with symbolic mode."""
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             json={"type": "file", "mode": "rw-rw-r--"},
                             content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['mode'] == '664'
        assert data['originalMode'] == 'rw-rw-r--'
        mock_create_uss_file.assert_called_once_with('/tmp/newfile.txt', '664')
    
    @patch('zusf_routes.zusf.create_uss_file')
    def test_create_uss_file_with_octal_mode(self, mock_create_uss_file, client):
        """Test creating file with octal mode."""
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             json={"type": "file", "mode": "600"},
                             content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['mode'] == '600'
        mock_create_uss_file.assert_called_once_with('/tmp/newfile.txt', '600')
    
    def test_create_uss_item_invalid_type(self, client):
        """Test creating item with invalid type."""
        response = client.post('/zosmf/restfiles/fs/tmp/newitem',
                             json={"type": "invalid"},
                             content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'type must be' in data['error']
    
    def test_create_uss_item_invalid_mode(self, client):
        """Test creating item with invalid mode."""
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             json={"type": "file", "mode": "invalid"},
                             content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'Invalid mode format' in data['error']
    
    def test_create_uss_item_no_json(self, client):
        """Test creating item without JSON content type."""
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             data="not json")
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'Content-Type must be application/json' in data['error']
    
    @patch('zusf_routes.zusf.create_uss_file')
    def test_create_uss_file_exception(self, mock_create_uss_file, client):
        """Test handling of exceptions during file creation."""
        mock_create_uss_file.side_effect = Exception("Directory does not exist")
        
        response = client.post('/zosmf/restfiles/fs/tmp/newfile.txt',
                             json={"type": "file"},
                             content_type='application/json')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not create USS file' in data['error']
        assert 'Directory does not exist' in data['details']


class TestDeleteUSSItem:
    """Test cases for the delete_uss_item endpoint."""
    
    @patch('zusf_routes.zusf.delete_uss_item')
    def test_delete_uss_item_success(self, mock_delete_uss_item, client):
        """Test successful deletion of a USS item."""
        response = client.delete('/zosmf/restfiles/fs/tmp/oldfile.txt')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['success'] == True
        assert data['path'] == '/tmp/oldfile.txt'
        assert data['recursive'] == False
        mock_delete_uss_item.assert_called_once_with('/tmp/oldfile.txt', False)
    
    @patch('zusf_routes.zusf.delete_uss_item')
    def test_delete_uss_item_recursive(self, mock_delete_uss_item, client):
        """Test recursive deletion of a USS item."""
        response = client.delete('/zosmf/restfiles/fs/tmp/olddir?recursive=true')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['recursive'] == True
        mock_delete_uss_item.assert_called_once_with('/tmp/olddir', True)
    
    @patch('zusf_routes.zusf.delete_uss_item')
    def test_delete_uss_item_exception(self, mock_delete_uss_item, client):
        """Test handling of exceptions during item deletion."""
        mock_delete_uss_item.side_effect = Exception("File not found")
        
        response = client.delete('/zosmf/restfiles/fs/tmp/nonexistent.txt')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'Failed to delete USS item' in data['error']
        assert 'File not found' in data['details']


class TestUSSFileUtilities:
    """Test cases for the uss_file_utilities endpoint."""
    
    @patch('zusf_routes.zusf.chmod_uss_item')
    def test_chmod_operation_success(self, mock_chmod_uss_item, client):
        """Test successful chmod operation."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chmod", "mode": "755"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['operation'] == 'chmod'
        assert data['mode'] == '755'
        assert data['recursive'] == False
        mock_chmod_uss_item.assert_called_once_with('/tmp/file.txt', '755', False)
    
    @patch('zusf_routes.zusf.chmod_uss_item')
    def test_chmod_operation_symbolic_mode(self, mock_chmod_uss_item, client):
        """Test chmod operation with symbolic mode."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chmod", "mode": "rwxr-xr-x", "recursive": True},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['mode'] == '755'
        assert data['originalMode'] == 'rwxr-xr-x'
        assert data['recursive'] == True
        mock_chmod_uss_item.assert_called_once_with('/tmp/file.txt', '755', True)
    
    @patch('zusf_routes.zusf.chown_uss_item')
    def test_chown_operation_success(self, mock_chown_uss_item, client):
        """Test successful chown operation."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chown", "owner": "testuser"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['operation'] == 'chown'
        assert data['owner'] == 'testuser'
        mock_chown_uss_item.assert_called_once_with('/tmp/file.txt', 'testuser', False)
    
    @patch('zusf_routes.zusf.chtag_uss_item')
    def test_chtag_operation_success(self, mock_chtag_uss_item, client):
        """Test successful chtag operation."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chtag", "tag": "UTF-8"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['operation'] == 'chtag'
        assert data['tag'] == 'UTF-8'
        mock_chtag_uss_item.assert_called_once_with('/tmp/file.txt', 'UTF-8', False)
    
    def test_chmod_missing_mode(self, client):
        """Test chmod operation with missing mode."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chmod"},
                            content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'mode is required for chmod operation' in data['error']
    
    def test_chown_missing_owner(self, client):
        """Test chown operation with missing owner."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chown"},
                            content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'owner is required for chown operation' in data['error']
    
    def test_chtag_missing_tag(self, client):
        """Test chtag operation with missing tag."""
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chtag"},
                            content_type='application/json')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'tag is required for chtag operation' in data['error']
    
    @patch('zusf_routes.zusf.write_uss_file')
    def test_fallback_to_file_write(self, mock_write_uss_file, client):
        """Test fallback to file write operation when no utility request specified."""
        mock_write_uss_file.return_value = "etag123"
        
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"records": "test content"},
                            content_type='application/json')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['success'] == True
        assert data['etag'] == "etag123"
        mock_write_uss_file.assert_called_once_with('/tmp/file.txt', 'test content', '', '')
    
    @patch('zusf_routes.zusf.chmod_uss_item')
    def test_utilities_exception(self, mock_chmod_uss_item, client):
        """Test handling of exceptions during utility operations."""
        mock_chmod_uss_item.side_effect = Exception("Permission denied")
        
        response = client.put('/zosmf/restfiles/fs/tmp/file.txt',
                            json={"request": "chmod", "mode": "755"},
                            content_type='application/json')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not perform chmod on USS path' in data['error']
        assert 'Permission denied' in data['details']


if __name__ == '__main__':
    pytest.main([__file__])