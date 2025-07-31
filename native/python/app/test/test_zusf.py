"""
Integration tests for zUSF (z/OS Unix System Files) Flask routes using pytest.
Tests all endpoints against the actual z/OS system.

Run with: pytest test_zusf_integration.py -v
"""

import requests
import json
import pytest
import urllib3
import time

# Disable SSL warnings for testing
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)


@pytest.fixture(scope="session")
def base_url():
    """Base URL for the zUSF service"""
    return "https://b025.lvn.broadcom.net:10190"


@pytest.fixture(scope="session")
def session():
    """HTTP session with SSL verification disabled"""
    session = requests.Session()
    session.verify = False
    return session


@pytest.fixture(scope="session")
def test_config():
    """Test configuration data"""
    return {
        "test_dir": "/tmp/pytest_test",
        "test_file": "testfile.txt",
        "test_content": "This is test content for integration testing.",
        "test_encoding": "UTF-8"
    }


@pytest.fixture(scope="session")
def test_directory(base_url, session, test_config):
    """Create test directory for the session"""
    dir_path = test_config["test_dir"]
    url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{dir_path}"
    
    # Create directory
    payload = {"type": "directory", "mode": "755"}
    response = session.post(url, json=payload, headers={"Content-Type": "application/json"})
    
    return dir_path


@pytest.fixture(scope="session")
def test_file_path(test_config):
    """Get the full test file path"""
    return f"{test_config['test_dir']}/{test_config['test_file']}"


class TestZUSFRoutes:
    """Test class for zUSF routes"""
    
    def test_list_uss_files(self, base_url, session, test_directory):
        """Test GET /zosmf/restfiles/fs - List USS files"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs"
        params = {"path": test_directory}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "items" in data
        assert "returnedRows" in data
        assert "path" in data
        assert data["path"] == test_directory
        assert isinstance(data["items"], list)
        assert data["returnedRows"] == len(data["items"])
    
    def test_create_uss_file(self, base_url, session, test_file_path):
        """Test POST /zosmf/restfiles/fs/<path> - Create USS file"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        session.delete(url)
        headers = {"Content-Type": "application/json"}
        payload = {"type": "file", "mode": "644"}
        
        response = session.post(url, json=payload, headers=headers)
        
        assert response.status_code == 201
        data = response.json()
        assert "success" in data
        assert "path" in data
        assert "type" in data
        assert data["success"] == True
        assert data["path"] == test_file_path
        assert data["type"] == "file"

        params = {"recursive": "true"}
        session.delete(url, params=params)
    
    def test_write_uss_file(self, base_url, session, test_file_path, test_config):
        """Test PUT /zosmf/restfiles/fs/<path> - Write to USS file"""
        # First create the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Now write to it
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "success" in data
        assert "etag" in data
        assert "filePath" in data
        assert data["success"] == True
        assert data["filePath"] == test_file_path
    
    def test_read_uss_file(self, base_url, session, test_file_path, test_config):
        """Test GET /zosmf/restfiles/fs/<path> - Read USS file"""
        # First create and write to the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        write_payload = {"records": test_config["test_content"]}
        session.put(create_url, json=write_payload, headers={"Content-Type": "application/json"})
        
        # Now read it
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "filePath" in data
        assert "format" in data
        assert data["filePath"] == test_file_path
        assert data["format"] == "text"
    
    def test_create_uss_directory(self, base_url, session, test_config):
        """Test POST /zosmf/restfiles/fs/<path> - Create USS directory"""
        subdir_path = f"{test_config['test_dir']}/subdir"
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{subdir_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"type": "directory", "mode": "755"}
        
        response = session.post(url, json=payload, headers=headers)
        
        assert response.status_code == 201
        data = response.json()
        assert "success" in data
        assert "path" in data
        assert "type" in data
        assert data["success"] == True
        assert data["path"] == subdir_path
        assert data["type"] == "directory"

        params = {"recursive": "true"}
        session.delete(url, params=params)
    
    def test_list_uss_files_with_options(self, base_url, session, test_directory):
        """Test GET /zosmf/restfiles/fs with all and long options"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs"
        params = {
            "path": test_directory,
            "all": "true",
            "long": "true"
        }
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "format" in data
        assert "showHidden" in data
        assert data["format"] == "long"
        assert data["showHidden"] == True
        
        # Check for warnings about unsupported parameters
        if "warnings" in data:
            assert len(data["warnings"]) >= 2
    
    def test_read_uss_file_with_encoding(self, base_url, session, test_file_path, test_config):
        """Test GET /zosmf/restfiles/fs/<path> with encoding parameter"""
        # First create and write to the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        write_payload = {"records": test_config["test_content"]}
        session.put(create_url, json=write_payload, headers={"Content-Type": "application/json"})
        
        # Now read with encoding
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        params = {"encoding": test_config["test_encoding"]}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "filePath" in data
    
    def test_read_uss_file_with_etag(self, base_url, session, test_file_path, test_config):
        """Test GET /zosmf/restfiles/fs/<path> with ETag parameter"""
        # First create and write to the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        write_payload = {"records": test_config["test_content"]}
        session.put(create_url, json=write_payload, headers={"Content-Type": "application/json"})
        
        # Now read with ETag
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        params = {"return-etag": "true"}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "etag" in data
        assert data["etag"] is not None
    
    def test_read_uss_file_bytes_format(self, base_url, session, test_file_path):
        """Test GET /zosmf/restfiles/fs/<path> in bytes format"""
        # First create and write to the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        write_payload = {"records": "Hi"}
        session.put(create_url, json=write_payload, headers={"Content-Type": "application/json"})
        
        # Now read in bytes format
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        params = {"response-format-bytes": "true"}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "format" in data
        assert "records" in data
        assert data["format"] == "bytes"
        assert isinstance(data["records"], list)
    
    
    def test_chmod_operation(self, base_url, session, test_file_path):
        """Test PUT /zosmf/restfiles/fs/<path> - chmod operation"""
        # First create the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Perform chmod operation
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"request": "chmod", "mode": "755"}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "operation" in data
        assert "mode" in data
        assert data["operation"] == "chmod"
        assert data["mode"] == "755"
    
    def test_chown_operation(self, base_url, session, test_file_path):
        """Test PUT /zosmf/restfiles/fs/<path> - chown operation"""
        # First create the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Perform chown operation
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"request": "chown", "owner": "testuser"}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "operation" in data
        assert "owner" in data
        assert data["operation"] == "chown"
        assert data["owner"] == "testuser"
    
    def test_chtag_operation(self, base_url, session, test_file_path):
        """Test PUT /zosmf/restfiles/fs/<path> - chtag operation"""
        # First create the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Perform chtag operation
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"request": "chtag", "tag": "EBCDIC"}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "operation" in data
        assert "tag" in data
        assert data["operation"] == "chtag"
    
    def test_delete_uss_file(self, base_url, session, test_file_path):
        """Test DELETE /zosmf/restfiles/fs/<path> - Delete USS file"""
        # First create the file
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        create_payload = {"type": "file", "mode": "644"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Now delete it
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        response = session.delete(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "success" in data
        assert "path" in data
        assert data["success"] == True
        assert data["path"] == test_file_path
    
    def test_delete_uss_directory_recursive(self, base_url, session, test_config):
        """Test DELETE /zosmf/restfiles/fs/<path> - Delete directory recursively"""
        # First create a subdirectory
        subdir_path = f"{test_config['test_dir']}/deleteme"
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{subdir_path}"
        create_payload = {"type": "directory", "mode": "755"}
        session.post(create_url, json=create_payload, headers={"Content-Type": "application/json"})
        
        # Now delete it recursively
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{subdir_path}"
        params = {"recursive": "true"}
        response = session.delete(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "success" in data
        assert "recursive" in data
        assert data["success"] == True
        assert data["recursive"] == True
    
    def test_list_uss_files_missing_path(self, base_url, session):
        """Test error handling for missing path parameter"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs"
        
        response = session.get(url)
        
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "path parameter is required" in data["error"]
    
    def test_create_uss_item_invalid_type(self, base_url, session, test_file_path):
        """Test error handling for invalid type parameter"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"type": "invalid"}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        # The exact error message may vary based on implementation
    
    def test_chmod_missing_mode(self, base_url, session, test_file_path):
        """Test error handling for chmod operation with missing mode"""

        url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{test_file_path}"
        headers = {"Content-Type": "application/json"}
        payload = {"request": "chmod", "mode":""}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "mode is required for chmod operation" in data["error"]