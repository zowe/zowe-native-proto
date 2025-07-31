"""
Integration tests for zDS (z/OS Data Set) Flask routes using pytest.
Tests all endpoints against the actual z/OS system.

Run with: pytest test_zds_integration.py -v
"""

import requests
import pytest
import urllib3

# Disable SSL warnings for testing
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)


@pytest.fixture(scope="session")
def base_url():
    """Base URL for the zDS service"""
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
        "dataset_prefix": "TEST",
        "test_dataset": "TEST.DATASET",
        "test_pds": "TEST.USER.TESTPDS",
        "test_member": "TESTMEM",
        "test_content": "This is test content for integration testing."
    }


@pytest.fixture(scope="session")
def test_dataset(base_url, session, test_config):
    """Create test dataset for the session"""
    dataset_name = test_config["test_dataset"]
    url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{dataset_name}"
    params = {
        "dsorg": "PS",
        "lrecl": "80",
        "primary": "1",
        "secondary": "1"
    }
    
    # Create dataset
    response = session.post(url, params=params)
    if response.status_code not in [200, 201]:
        pytest.skip(f"Could not create test dataset: {response.text}")
    
    yield dataset_name
    
    # Cleanup - delete dataset
    try:
        session.delete(url)
    except:
        pass  # Ignore cleanup errors


@pytest.fixture(scope="session")
def test_pds(base_url, session, test_config):
    """Create test PDS for the session"""
    pds_name = test_config["test_pds"]
    url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{pds_name}"
    params = {
        "dsorg": "PO",
        "recfm": "FB",
        "lrecl": "80",
        "primary": "1",
        "secondary": "1",
        "dirblk": "10"
    }
    
    # Create PDS
    response = session.post(url, params=params)
    if response.status_code not in [200, 201]:
        pytest.skip(f"Could not create test PDS: {response.text}")
    
    yield pds_name
    
    # Cleanup - delete PDS
    try:
        session.delete(url)
    except:
        pass  # Ignore cleanup errors


class TestZDSRoutes:
    """Test class for zDS routes"""
    
    def test_list_data_sets(self, base_url, session, test_config):
        """Test GET /zosmf/restfiles/ds - List data sets"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds"
        params = {"dslevel": test_config["dataset_prefix"]}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "items" in data
        assert "returnedRows" in data
        assert isinstance(data["items"], list)
        assert data["returnedRows"] == len(data["items"])
    
    def test_create_data_set(self, base_url, session):
        """Test POST /zosmf/restfiles/ds/<name> - Create data set"""
        dataset_name = "TEST.USER.PYTEST01"
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{dataset_name}"
        params = {
            "dsorg": "PS",
            "lrecl": "80",
            "primary": "1",
            "secondary": "1"
        }
        
        response = session.post(url, params=params)
        
        assert response.status_code == 201
        data = response.json()
        assert "message" in data
        assert "datasetName" in data
        assert data["datasetName"] == dataset_name
        assert "created" in data["message"].lower()
        
        # Cleanup
        session.delete(url)
    
    def test_write_data_set(self, base_url, session, test_dataset, test_config):
        """Test PUT /zosmf/restfiles/ds/<name> - Write to data set"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_dataset}"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "message" in data
        assert "etag" in data
        assert data["datasetName"] == test_dataset
    
    def test_read_data_set(self, base_url, session, test_dataset, test_config):
        """Test GET /zosmf/restfiles/ds/<name> - Read data set"""
        # First write some data
        write_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_dataset}"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        session.put(write_url, json=payload, headers=headers)
        
        # Now read it
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_dataset}"
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "datasetName" in data
        assert data["datasetName"] == test_dataset
        assert "format" in data
    
    def test_create_pds(self, base_url, session):
        """Test POST /zosmf/restfiles/ds/<name> - Create PDS"""
        pds_name = "TEST.USER.PYTEST02"
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{pds_name}"
        params = {
            "dsorg": "PO",
            "recfm": "FB",
            "lrecl": "80",
            "primary": "1",
            "secondary": "1",
            "dirblk": "10"
        }
        
        response = session.post(url, params=params)
        
        assert response.status_code == 201
        data = response.json()
        assert "message" in data
        assert "datasetName" in data
        assert data["datasetName"] == pds_name
        
        # Cleanup
        session.delete(url)
    
    def test_create_member(self, base_url, session, test_pds, test_config):
        """Test POST /zosmf/restfiles/ds/<dataset>/<member> - Create member"""
        member_name = test_config["test_member"]
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/{member_name}"
        
        response = session.post(url)
        
        assert response.status_code == 201
        data = response.json()
        assert "message" in data
        assert "memberName" in data
        assert data["memberName"] == member_name
        assert data["datasetName"] == test_pds
    
    def test_list_members(self, base_url, session, test_pds, test_config):
        """Test GET /zosmf/restfiles/ds/<dataset>/member - List members"""
        # First create a member
        member_name = test_config["test_member"]
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/{member_name}"
        session.post(create_url)
        
        # Now list members
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/member"
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "items" in data
        assert "returnedRows" in data
        assert "datasetName" in data
        assert data["datasetName"] == test_pds
        
        # Should find our test member
        member_names = [item["name"] for item in data["items"]]
        assert member_name in member_names
    
    def test_write_member(self, base_url, session, test_pds, test_config):
        """Test PUT /zosmf/restfiles/ds/<dataset>(<member>) - Write to member"""
        member_name = test_config["test_member"]
        
        # First create the member
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/{member_name}"
        session.post(create_url)
        
        # Now write to it
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}({member_name})"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 200
        data = response.json()
        assert "message" in data
        assert member_name in data["datasetName"]
    
    def test_read_member(self, base_url, session, test_pds, test_config):
        """Test GET /zosmf/restfiles/ds/<dataset>(<member>) - Read member"""
        member_name = test_config["test_member"]
        
        # First create and write to the member
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/{member_name}"
        session.post(create_url)
        
        write_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}({member_name})"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        session.put(write_url, json=payload, headers=headers)
        
        # Now read it
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}({member_name})"
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "memberName" in data
        assert data["memberName"] == member_name
        assert data["datasetName"] == test_pds
    
    def test_delete_member(self, base_url, session, test_pds, test_config):
        """Test DELETE /zosmf/restfiles/ds/<dataset>(<member>) - Delete member"""
        member_name = test_config["test_member"]
        
        # First create the member
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}/{member_name}"
        session.post(create_url)
        
        # Now delete it
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_pds}({member_name})"
        response = session.delete(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "message" in data
        assert "memberName" in data
        assert data["memberName"] == member_name
        assert "deleted" in data["message"].lower()
    
    def test_delete_data_set(self, base_url, session):
        """Test DELETE /zosmf/restfiles/ds/<name> - Delete data set"""
        # First create a dataset
        dataset_name = "TEST.USER.PYTEST03"
        create_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{dataset_name}"
        params = {
            "dsorg": "PS",
            "recfm": "FB",
            "lrecl": "80",
            "primary": "1",
            "secondary": "1"
        }
        session.post(create_url, params=params)
        
        # Now delete it
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{dataset_name}"
        response = session.delete(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "message" in data
        assert "datasetName" in data
        assert data["datasetName"] == dataset_name
        assert "deleted" in data["message"].lower()
    
    def test_list_data_sets_with_attributes(self, base_url, session, test_config):
        """Test GET /zosmf/restfiles/ds with attributes parameter"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds"
        params = {
            "dslevel": test_config["dataset_prefix"],
            "attributes": "true"
        }
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "items" in data
        
        if data["items"]:
            first_item = data["items"][0]
            expected_fields = ["name", "volser", "dsorg", "recfm", "migr"]
            for field in expected_fields:
                assert field in first_item
    
    def test_read_data_set_with_etag(self, base_url, session, test_dataset, test_config):
        """Test GET /zosmf/restfiles/ds with ETag parameter"""
        # First write some data
        write_url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_dataset}"
        headers = {"Content-Type": "application/json"}
        payload = {"records": test_config["test_content"]}
        session.put(write_url, json=payload, headers=headers)
        
        # Now read with ETag
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{test_dataset}"
        params = {"return-etag": "true"}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "etag" in data
        assert len(data["etag"]) == 32  # MD5 hash length
    
    def test_invalid_dslevel(self, base_url, session):
        """Test error handling for invalid dslevel parameter"""
        url = f"{base_url}/pythonservice/zosmf/restfiles/ds"
        
        # Test missing dslevel
        response = session.get(url)
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "dslevel parameter is required" in data["error"]
        
        # Test dslevel too long (>44 characters)
        long_dslevel = "A" * 45
        params = {"dslevel": long_dslevel}
        response = session.get(url, params=params)
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "exceeds 44 character length limit" in data["error"]