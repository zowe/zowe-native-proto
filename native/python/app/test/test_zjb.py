"""
Simplified test suite for zJB (z/OS Job) Flask routes.
Tests core functionality for job operations.
"""

import pytest
import json
import sys
import os
from unittest.mock import patch, MagicMock
from flask import Flask

# Add the parent directory to the Python path to import from routes/
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from routes import zjb_bp


@pytest.fixture
def app():
    """Create a Flask test application."""
    app = Flask(__name__)
    app.register_blueprint(zjb_bp)
    app.config['TESTING'] = True
    return app


@pytest.fixture
def client(app):
    """Create a test client for the Flask application."""
    return app.test_client()


@pytest.fixture
def mock_job_info():
    """Create a mock job info object."""
    job = MagicMock()
    job.jobname = "TESTJOB"
    job.jobid = "JOB12345"
    job.owner = "TESTUSER"
    job.status = "OUTPUT"
    job.retcode = "CC 0000"
    job.job_correlator = "CORR123"
    job.full_status = "COMPLETED"
    return job


@pytest.fixture
def mock_spool_file():
    """Create a mock spool file object."""
    spool = MagicMock()
    spool.ddn = "SYSPRINT"
    spool.dsn = "TESTJOB.SYSPRINT"
    spool.key = 101
    spool.stepname = "STEP1"
    spool.procstep = "PROC1"
    return spool


class TestGetJobStatusByNameAndId:
    """Test cases for the get_job_status_by_name_and_id endpoint."""
    
    @patch('routes.zjb_routes.zjb.get_job_status')
    def test_get_job_status_success(self, mock_get_job_status, client, mock_job_info):
        """Test successful job status retrieval by name and ID."""
        mock_get_job_status.return_value = mock_job_info
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['status'] == 'OUTPUT'
        assert data['retcode'] == 'CC 0000'
        assert data['type'] == 'JOB'
        mock_get_job_status.assert_called_once_with('JOB12345')
    
    def test_get_job_status_missing_params(self, client):
        """Test job status retrieval with missing parameters."""
        response = client.get('/zosmf/restjobs/jobs//')
        
        assert response.status_code == 404  # Route not found
    
    @patch('routes.zjb_routes.zjb.get_job_status')
    def test_get_job_status_exception(self, mock_get_job_status, client):
        """Test handling of exceptions during job status retrieval."""
        mock_get_job_status.side_effect = Exception("Job not found")
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not get job status' in data['error']


class TestGetJobStatusByCorrelator:
    """Test cases for the get_job_status_by_correlator endpoint."""
    
    @patch('routes.zjb_routes.zjb.get_job_status')
    def test_get_job_status_by_correlator_success(self, mock_get_job_status, client, mock_job_info):
        """Test successful job status retrieval by correlator."""
        mock_get_job_status.return_value = mock_job_info
        
        response = client.get('/zosmf/restjobs/jobs/CORR123')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['job-correlator'] == 'CORR123'
        assert data['type'] == 'JOB'
        mock_get_job_status.assert_called_once_with('CORR123')
    
    @patch('routes.zjb_routes.zjb.get_job_status')
    def test_get_job_status_by_correlator_exception(self, mock_get_job_status, client):
        """Test handling of exceptions during job status retrieval by correlator."""
        mock_get_job_status.side_effect = Exception("Correlator not found")
        
        response = client.get('/zosmf/restjobs/jobs/CORR123')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not get job status' in data['error']


class TestListJobs:
    """Test cases for the list_jobs endpoint."""
    
    @patch('routes.zjb_routes.zjb.list_jobs_by_owner')
    def test_list_jobs_success(self, mock_list_jobs, client, mock_job_info):
        """Test successful job listing."""
        mock_list_jobs.return_value = [mock_job_info]
        
        response = client.get('/zosmf/restjobs/jobs?owner=TESTUSER')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 1
        assert data['owner'] == 'TESTUSER'
        assert data['items'][0]['jobname'] == 'TESTJOB'
        assert data['items'][0]['type'] == 'JOB'
        mock_list_jobs.assert_called_once_with('TESTUSER')
    
    @patch('routes.zjb_routes.zjb.list_jobs_by_owner')
    def test_list_jobs_default_owner(self, mock_list_jobs, client, mock_job_info):
        """Test job listing with default owner."""
        mock_list_jobs.return_value = [mock_job_info]
        
        response = client.get('/zosmf/restjobs/jobs')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['owner'] == '*'
        mock_list_jobs.assert_called_once_with('*')
    
    @patch('routes.zjb_routes.zjb.list_jobs_by_owner')
    def test_list_jobs_exception(self, mock_list_jobs, client):
        """Test handling of exceptions during job listing."""
        mock_list_jobs.side_effect = Exception("System error")
        
        response = client.get('/zosmf/restjobs/jobs')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not list jobs' in data['error']


class TestListJobFilesByNameAndId:
    """Test cases for the list_job_files_by_name_and_id endpoint."""
    
    @patch('routes.zjb_routes.zjb.list_spool_files')
    def test_list_job_files_success(self, mock_list_spool_files, client, mock_spool_file):
        """Test successful job file listing by name and ID."""
        mock_list_spool_files.return_value = [mock_spool_file]
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 1
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['items'][0]['ddname'] == 'SYSPRINT'
        assert data['items'][0]['id'] == 101
        mock_list_spool_files.assert_called_once_with('JOB12345')
    
    @patch('routes.zjb_routes.zjb.list_spool_files')
    def test_list_job_files_exception(self, mock_list_spool_files, client):
        """Test handling of exceptions during job file listing."""
        mock_list_spool_files.side_effect = Exception("Job not found")
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not list files' in data['error']


class TestListJobFilesByCorrelator:
    """Test cases for the list_job_files_by_correlator endpoint."""
    
    @patch('routes.zjb_routes.zjb.list_spool_files')
    def test_list_job_files_by_correlator_success(self, mock_list_spool_files, client, mock_spool_file):
        """Test successful job file listing by correlator."""
        mock_list_spool_files.return_value = [mock_spool_file]
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['returnedRows'] == 1
        assert data['job-correlator'] == 'CORR123'
        assert data['items'][0]['ddname'] == 'SYSPRINT'
        mock_list_spool_files.assert_called_once_with('CORR123')
    
    @patch('routes.zjb_routes.zjb.list_spool_files')
    def test_list_job_files_by_correlator_exception(self, mock_list_spool_files, client):
        """Test handling of exceptions during job file listing by correlator."""
        mock_list_spool_files.side_effect = Exception("Correlator not found")
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not list files' in data['error']


class TestReadJobFileByNameAndId:
    """Test cases for the read_job_file_by_name_and_id endpoint."""
    
    @patch('routes.zjb_routes.zjb.read_spool_file')
    def test_read_job_file_success(self, mock_read_spool_file, client):
        """Test successful job file reading by name and ID."""
        mock_read_spool_file.return_value = "Job output content"
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files/101/records')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "Job output content"
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['id'] == 101
        assert data['format'] == 'text'
        mock_read_spool_file.assert_called_once_with('JOB12345', 101)
    
    def test_read_job_file_invalid_file_id(self, client):
        """Test job file reading with invalid file ID."""
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files/invalid/records')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'file_id must be a number' in data['error']
    
    @patch('routes.zjb_routes.zjb.read_spool_file')
    def test_read_job_file_exception(self, mock_read_spool_file, client):
        """Test handling of exceptions during job file reading."""
        mock_read_spool_file.side_effect = Exception("File not found")
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files/101/records')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not view job file' in data['error']


class TestReadJobFileByCorrelator:
    """Test cases for the read_job_file_by_correlator endpoint."""
    
    @patch('routes.zjb_routes.zjb.read_spool_file')
    def test_read_job_file_by_correlator_success(self, mock_read_spool_file, client):
        """Test successful job file reading by correlator."""
        mock_read_spool_file.return_value = "Job output content"
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files/101/records')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "Job output content"
        assert data['job-correlator'] == 'CORR123'
        assert data['id'] == 101
        assert data['format'] == 'text'
        mock_read_spool_file.assert_called_once_with('CORR123', 101)
    
    @patch('routes.zjb_routes.zjb.read_spool_file')
    def test_read_job_file_by_correlator_exception(self, mock_read_spool_file, client):
        """Test handling of exceptions during job file reading by correlator."""
        mock_read_spool_file.side_effect = Exception("File not found")
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files/101/records')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not view job file' in data['error']


class TestReadJobJCLByNameAndId:
    """Test cases for the read_job_jcl_by_name_and_id endpoint."""
    
    @patch('routes.zjb_routes.zjb.get_job_jcl')
    def test_read_job_jcl_success(self, mock_get_job_jcl, client):
        """Test successful JCL reading by name and ID."""
        mock_get_job_jcl.return_value = "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files/JCL/records')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['ddname'] == 'JCL'
        assert data['type'] == 'JCL'
        mock_get_job_jcl.assert_called_once_with('JOB12345')
    
    @patch('routes.zjb_routes.zjb.get_job_jcl')
    def test_read_job_jcl_exception(self, mock_get_job_jcl, client):
        """Test handling of exceptions during JCL reading."""
        mock_get_job_jcl.side_effect = Exception("JCL not found")
        
        response = client.get('/zosmf/restjobs/jobs/TESTJOB/JOB12345/files/JCL/records')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not view JCL' in data['error']


class TestReadJobJCLByCorrelator:
    """Test cases for the read_job_jcl_by_correlator endpoint."""
    
    @patch('routes.zjb_routes.zjb.get_job_jcl')
    def test_read_job_jcl_by_correlator_success(self, mock_get_job_jcl, client):
        """Test successful JCL reading by correlator."""
        mock_get_job_jcl.return_value = "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files/JCL/records')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['records'] == "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        assert data['job-correlator'] == 'CORR123'
        assert data['ddname'] == 'JCL'
        assert data['type'] == 'JCL'
        mock_get_job_jcl.assert_called_once_with('CORR123')
    
    @patch('routes.zjb_routes.zjb.get_job_jcl')
    def test_read_job_jcl_by_correlator_exception(self, mock_get_job_jcl, client):
        """Test handling of exceptions during JCL reading by correlator."""
        mock_get_job_jcl.side_effect = Exception("JCL not found")
        
        response = client.get('/zosmf/restjobs/jobs/CORR123/files/JCL/records')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not view JCL' in data['error']


class TestSubmitJob:
    """Test cases for the submit_job endpoint."""
    
    @patch('routes.zjb_routes.zjb.submit_job')
    def test_submit_job_text_content(self, mock_submit_job, client):
        """Test successful job submission with text content."""
        mock_submit_job.return_value = "JOB12345"
        
        response = client.put('/zosmf/restjobs/jobs',
                            data="//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14",
                            content_type='text/plain')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['jobid'] == 'JOB12345'
        assert data['success'] == True
        assert 'Submitted JCL content' in data['message']
        mock_submit_job.assert_called_once_with("//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14")
    
    @patch('routes.zjb_routes.zjb.submit_job')
    @patch('routes.zjb_routes.zds.read_data_set')
    def test_submit_job_from_dataset(self, mock_read_data_set, mock_submit_job, client):
        """Test successful job submission from data set."""
        mock_read_data_set.return_value = "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        mock_submit_job.return_value = "JOB12345"
        
        response = client.put('/zosmf/restjobs/jobs',
                            json={"dsn": "USER.TEST.JCL"},
                            content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['jobid'] == 'JOB12345'
        assert data['success'] == True
        mock_read_data_set.assert_called_once_with("USER.TEST.JCL", "")
        mock_submit_job.assert_called_once_with("//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14")
    
    @patch('routes.zjb_routes.zjb.submit_job')
    @patch('routes.zjb_routes.zusf.read_uss_file')
    def test_submit_job_from_uss_file(self, mock_read_uss_file, mock_submit_job, client):
        """Test successful job submission from USS file."""
        mock_read_uss_file.return_value = "//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14"
        mock_submit_job.return_value = "JOB12345"
        
        response = client.put('/zosmf/restjobs/jobs',
                            json={"file": "/tmp/test.jcl"},
                            content_type='application/json')
        
        assert response.status_code == 201
        data = json.loads(response.data)
        assert data['jobid'] == 'JOB12345'
        assert data['success'] == True
        mock_read_uss_file.assert_called_once_with("/tmp/test.jcl", "")
        mock_submit_job.assert_called_once_with("//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14")
    
    def test_submit_job_no_content(self, client):
        """Test job submission with no content."""
        response = client.put('/zosmf/restjobs/jobs',
                            data="",
                            content_type='text/plain')
        
        assert response.status_code == 400
        data = json.loads(response.data)
        assert 'JCL content is required' in data['error']
    
    @patch('routes.zjb_routes.zjb.submit_job')
    def test_submit_job_exception(self, mock_submit_job, client):
        """Test handling of exceptions during job submission."""
        mock_submit_job.side_effect = Exception("Submission failed")
        
        response = client.put('/zosmf/restjobs/jobs',
                            data="//TESTJOB JOB\n//STEP1 EXEC PGM=IEFBR14",
                            content_type='text/plain')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not submit JCL' in data['error']


class TestDeleteJobByNameAndId:
    """Test cases for the delete_job_by_name_and_id endpoint."""
    
    @patch('routes.zjb_routes.zjb.delete_job')
    def test_delete_job_success(self, mock_delete_job, client):
        """Test successful job deletion by name and ID."""
        response = client.delete('/zosmf/restjobs/jobs/TESTJOB/JOB12345')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['jobname'] == 'TESTJOB'
        assert data['jobid'] == 'JOB12345'
        assert data['success'] == True
        assert 'deleted' in data['message']
        mock_delete_job.assert_called_once_with('JOB12345')
    
    @patch('routes.zjb_routes.zjb.delete_job')
    def test_delete_job_exception(self, mock_delete_job, client):
        """Test handling of exceptions during job deletion."""
        mock_delete_job.side_effect = Exception("Deletion failed")
        
        response = client.delete('/zosmf/restjobs/jobs/TESTJOB/JOB12345')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not delete job' in data['error']


class TestDeleteJobByCorrelator:
    """Test cases for the delete_job_by_correlator endpoint."""
    
    @patch('routes.zjb_routes.zjb.delete_job')
    def test_delete_job_by_correlator_success(self, mock_delete_job, client):
        """Test successful job deletion by correlator."""
        response = client.delete('/zosmf/restjobs/jobs/CORR123')
        
        assert response.status_code == 200
        data = json.loads(response.data)
        assert data['job-correlator'] == 'CORR123'
        assert data['success'] == True
        assert 'deleted' in data['message']
        mock_delete_job.assert_called_once_with('CORR123')
    
    @patch('routes.zjb_routes.zjb.delete_job')
    def test_delete_job_by_correlator_exception(self, mock_delete_job, client):
        """Test handling of exceptions during job deletion by correlator."""
        mock_delete_job.side_effect = Exception("Deletion failed")
        
        response = client.delete('/zosmf/restjobs/jobs/CORR123')
        
        assert response.status_code == 500
        data = json.loads(response.data)
        assert 'could not delete job' in data['error']


if __name__ == '__main__':
    pytest.main([__file__])