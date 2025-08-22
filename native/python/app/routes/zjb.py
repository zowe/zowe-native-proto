"""
Integration tests for zJB (z/OS Job) Flask routes using pytest.
Tests all endpoints against the actual z/OS system.

Run with: pytest test_zjb_integration.py -v
"""

import requests
import json
import pytest
import urllib3

# Disable SSL warnings for testing
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)


@pytest.fixture(scope="session")
def base_url():
    """Base URL for the zJB service"""
    return "https://b025.lvn.broadcom.net:10190"


@pytest.fixture(scope="session")
def session():
    """HTTP session with SSL verification disabled"""
    session = requests.Session()
    session.verify = False
    return session


@pytest.fixture(scope="session")
def test_config():
    """Static test configuration data"""
    return {
        "test_dataset": "TEST.JCL",
        "test_uss_file": "/tmp/test.jcl",
        "test_jcl_content": """//PYTEST   JOB  CLASS=A,MSGCLASS=H,NOTIFY=&SYSUID
//STEP1    EXEC PGM=IEFBR14
//"""
    }

@pytest.fixture(scope="session")
def test_data_set_jcl(base_url, session, test_config):
    ds = test_config["test_dataset"]
    url = f"{base_url}/pythonservice/zosmf/restfiles/ds/{ds}"
    params = {
        "dsorg": "PS",      
        "recfm": "FB",      
        "lrecl": "80",
        "primary": "10",
        "secondary": "5",
        "blksize": "3200"
    }
    response = session.post(url, params=params, verify=False)

    headers = {"Content-Type": "application/json"}
    payload = {"records": test_config["test_jcl_content"]}
    response = session.put(url, json=payload, headers=headers)

    yield ds

    session.delete(url)

@pytest.fixture(scope="session")
def test_uss_jcl(base_url, session, test_config):
    uss = test_config["test_uss_file"]
    url = f"{base_url}/pythonservice/zosmf/restfiles/fs/{uss}"
    create_payload = {"type": "file", "mode": "644"}
    session.post(url, json=create_payload, headers={"Content-Type": "application/json"})

    write_payload = {"records": test_config["test_jcl_content"]}
    session.put(url, json=write_payload, headers={"Content-Type": "application/json"})

    yield uss

    session.delete(url)

@pytest.fixture(scope="session")
def submitted_job(base_url, session, test_config):
    """Submit a real job and return its attributes for testing"""
    url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
    headers = {"Content-Type": "text/plain"}
    
    # Submit the test job
    response = session.put(url, data=test_config["test_jcl_content"], headers=headers)
    
    submit_data = response.json()
    jobid = submit_data["jobid"]
    
    # Get job status to extract all attributes
    status_url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobid}"
    status_response = session.get(status_url)
    submitted_job_info = status_response.json()
    
    # Return the actual job attributes
    job_attributes = {
        "jobname": submitted_job_info["jobname"],
        "jobid": submitted_job_info["jobid"],
        "owner": submitted_job_info["owner"],
        "job_correlator": submitted_job_info.get("job-correlator", ""),
        "status": submitted_job_info.get("status", "")
    }
    
    yield job_attributes
    
    # Cleanup - try to delete the job (ignore errors)
    try:
        delete_url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{job_attributes['jobname']}/{job_attributes['jobid']}"
        session.delete(delete_url)
    except:
        pass  # Ignore cleanup errors


class TestZJBRoutes:
    """Test class for zJB routes"""
    
    def test_get_job_status_by_name_and_id(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<jobname>/<jobid> - Get job status by name and ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "jobname" in data
        assert "jobid" in data
        assert "status" in data
        assert "type" in data
        assert data["type"] == "JOB"
    
    def test_get_job_status_by_correlator(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<correlator> - Get job status by correlator"""
        correlator = submitted_job["job_correlator"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{correlator}"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "jobname" in data
        assert "job-correlator" in data
        assert "status" in data
        assert "type" in data
        assert data["type"] == "JOB"
    
    def test_list_jobs(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs - List jobs"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        params = {"owner": submitted_job["owner"]}
        
        response = session.get(url, params=params)
        
        assert response.status_code == 200
        data = response.json()
        assert "returnedRows" in data
        assert "owner" in data
        assert "items" in data
        assert isinstance(data["items"], list)
        assert data["returnedRows"] == len(data["items"])
        assert data["owner"] == submitted_job["owner"]
    
    def test_list_job_files_by_name_and_id(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<jobname>/<jobid>/files - List job files by name and ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}/files"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "returnedRows" in data
        assert "jobname" in data
        assert "jobid" in data
        assert "items" in data
        assert data["jobname"] == jobname
        assert data["jobid"] == jobid
        assert isinstance(data["items"], list)
    
    def test_list_job_files_by_correlator(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<correlator>/files - List job files by correlator"""
        correlator = submitted_job["job_correlator"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{correlator}/files"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "returnedRows" in data
        assert "job-correlator" in data
        assert "items" in data
        assert data["job-correlator"] == correlator
        assert isinstance(data["items"], list)
    
    def test_read_job_file_by_name_and_id(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<jobname>/<jobid>/files/<id>/records - Read job file by name and ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        file = session.get(f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobid}/files").json()
        file_id = file["items"][0]['id']
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}/files/{file_id}/records"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "jobname" in data
        assert "jobid" in data
        assert "id" in data
        assert "format" in data
        assert data["jobname"] == jobname
        assert data["jobid"] == jobid
        assert data["id"] == file_id
    
    def test_read_job_file_by_correlator(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<correlator>/files/<id>/records - Read job file by correlator"""
        correlator = submitted_job["job_correlator"]
        file = session.get(f"{base_url}/pythonservice/zosmf/restjobs/jobs/{correlator}/files").json()
        file_id = file["items"][0]['id']
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{correlator}/files/{file_id}/records"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "job-correlator" in data
        assert "id" in data
        assert "format" in data
        assert data["job-correlator"] == correlator
        assert data["id"] == file_id
    
    def test_read_job_jcl_by_name_and_id(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<jobname>/<jobid>/files/JCL/records - Read JCL by name and ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}/files/JCL/records"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "jobname" in data
        assert "jobid" in data
        assert "ddname" in data
        assert "type" in data
        assert data["jobname"] == jobname
        assert data["jobid"] == jobid
        assert data["ddname"] == "JCL"
        assert data["type"] == "JCL"
    
    def test_read_job_jcl_by_correlator(self, base_url, session, submitted_job):
        """Test GET /zosmf/restjobs/jobs/<correlator>/files/JCL/records - Read JCL by correlator"""
        correlator = submitted_job["job_correlator"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{correlator}/files/JCL/records"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "records" in data
        assert "job-correlator" in data
        assert "ddname" in data
        assert "type" in data
        assert data["job-correlator"] == correlator
        assert data["ddname"] == "JCL"
        assert data["type"] == "JCL"
    
    def test_submit_job_text_content(self, base_url, session, test_config):
        """Test PUT /zosmf/restjobs/jobs - Submit job with text content"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        headers = {"Content-Type": "text/plain"}
        
        response = session.put(url, data=test_config["test_jcl_content"], headers=headers)
        
        assert response.status_code == 201
        data = response.json()
        assert "jobid" in data
        assert "success" in data
        assert "message" in data
        assert data["success"] == True
        assert "JOB" in data["jobid"]
    
    def test_submit_job_from_dataset(self, base_url, session, test_data_set_jcl):
        """Test PUT /zosmf/restjobs/jobs - Submit job from data set"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        headers = {"Content-Type": "application/json"}
        payload = {"dsn": test_data_set_jcl}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 201
        data = response.json()
        assert "jobid" in data
        assert "success" in data
        assert "message" in data
        assert data["success"] == True
        assert "JOB" in data["jobid"]
    
    def test_submit_job_from_uss_file(self, base_url, session, test_uss_jcl):
        """Test PUT /zosmf/restjobs/jobs - Submit job from USS file"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        headers = {"Content-Type": "application/json"}
        payload = {"file": test_uss_jcl}
        
        response = session.put(url, json=payload, headers=headers)
        
        assert response.status_code == 201
        data = response.json()
        assert "jobid" in data
        assert "success" in data
        assert "message" in data
        assert data["success"] == True
        assert "JOB" in data["jobid"]
    
    def test_delete_job_by_name_and_id(self, base_url, session, submitted_job):
        """Test DELETE /zosmf/restjobs/jobs/<jobname>/<jobid> - Delete job by name and ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}"
        
        response = session.delete(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "jobname" in data
        assert "jobid" in data
        assert "success" in data
        assert "message" in data
        assert data["jobname"] == jobname
        assert data["jobid"] == jobid
        assert data["success"] == True
        assert "deleted" in data["message"].lower()
    
    def test_list_jobs_default_owner(self, base_url, session):
        """Test GET /zosmf/restjobs/jobs with default owner"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        
        response = session.get(url)
        
        assert response.status_code == 200
        data = response.json()
        assert "returnedRows" in data
        assert "owner" in data
        assert "items" in data
        assert data["owner"] == "*"
        assert isinstance(data["items"], list)
    
    def test_invalid_file_id(self, base_url, session, submitted_job):
        """Test error handling for invalid file ID"""
        jobname = submitted_job["jobname"]
        jobid = submitted_job["jobid"]
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs/{jobname}/{jobid}/files/invalid/records"
        
        response = session.get(url)
        
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "file_id must be a number" in data["error"]
    
    def test_submit_job_empty_content(self, base_url, session):
        """Test error handling for empty JCL content"""
        url = f"{base_url}/pythonservice/zosmf/restjobs/jobs"
        headers = {"Content-Type": "text/plain"}
        
        response = session.put(url, data="", headers=headers)
        
        assert response.status_code == 400
        data = response.json()
        assert "error" in data
        assert "JCL content is required" in data["error"]