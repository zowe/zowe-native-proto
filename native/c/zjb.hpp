/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef ZJB_HPP
#define ZJB_HPP

#include <iostream>
#include <vector>
#include <string>
#include "zjbtype.h"

struct ZJob
{
  std::string jobname;
  std::string jobid;
  std::string owner;
  std::string status;
  std::string retcode;
};

struct ZJobDD
{
  std::string jobid;
  std::string ddn;
  std::string dsn;
  std::string stepname;
  std::string procstep;
  int key;
};

/**
 * @brief Return a list of jobs from an input or default owner
 *
 * @param zjb job returned attributes and error information
 * @param owner_name owner name of the job to query, defaults to currnet user if == "", may use wild cards, i.e. "IBMUS*"
 * @param jobs populated list returned containing job information array
 * @return int 0 for success; non zero otherwise
 */
int zjb_list_by_owner(ZJB *zjb, std::string owner_name, std::vector<ZJob> &jobs);

/**
 * @brief Return a list of jobs from an input or default owner
 *
 * @param zjb job returned attributes and error information
 * @param owner_name owner name of the job to query, defaults to currnet user if == "", may use wild cards, i.e. "IBMUS*"
 * @param prefix job prefix, defaults to "*" if == "", may use wild cards, i.e. "IBMUS*"
 * @param jobs populated list returned containing job information array
 * @return int 0 for success; non zero otherwise
 */
int zjb_list_by_owner(ZJB *zjb, std::string owner_name, std::string prefix_name, std::vector<ZJob> &jobs);

/**
 * @brief Return a job status struct from input jobid
 *
 * @param zjb job returned attributes and error information
 * @param jobid jobid used to search
 * @param job populated struct returned for found job
 * @return int 0 for success; non zero otherwise
 */
int zjb_view_by_jobid(ZJB *zjb, std::string jobid, ZJob &job);

/**
 * @brief Return a list of job file information from an input jobid
 *
 * @param zjb job returned attributes and error information
 * @param job_dds populated list returned containing job file information array
 * @return int 0 for success; non zero otherwise
 */
int zjb_list_dds_by_jobid(ZJB *zjb, std::string, std::vector<ZJobDD> &job_dds);

/**
 * @brief Return output from a specific job file
 *
 * @param zjb job returned attributes and error information
 * @param jobid jobid coresponding to the key to locate
 * @param key data set key returned from zjb_list_dds_by_jobid, e.g. JESMSGLG is usually 2
 * @param response return job file output
 * @return int 0 for success; non zero otherwise
 */
int zjb_read_jobs_output_by_jobid_and_key(ZJB *zjb, std::string jobid, int key, std::string &response);

int zjb_get_job_dsn_by_jobid_and_key(ZJB *zjb, std::string, int, std::string &);
int zjb_read_job_content_by_dsn(ZJB *zjb, std::string job_dsn, std::string &response);

/**
 * @brief Return JCL for a job by input jobid
 *
 * @param zjb job returned attributes and error information
 * @param jobid jobid coresponding to the key to locate
 * @param response return JCL
 * @return int 0 for success; non zero otherwise
 */
int zjb_read_job_jcl_by_jobid(ZJB *zjb, std::string jobid, std::string &response);

/**
 * @brief Submit a job from a given input data set
 *
 * @param zjb job returned attributes and error information
 * @param dsn data set name containing JCL to submit, i.e. "IBMUSER.JCL(IEFBR14)""
 * @param jobid jobid retuned after successfully submitting JCL
 * @return int 0 for success; non zero otherwise
 */
int zjb_submit(ZJB *zjb, std::string dsn, std::string &jobid);

/**
 * @brief Delete a job using input jobid
 *
 * @param zjb job returned attributes and error information
 * @param jobid jobid to delete, i.e. JOB00123 or J123
 * @return int 0 for success; non zero otherwise
 */
int zjb_delete_by_jobid(ZJB *zjb, std::string jobid);

#endif
