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

import "./setup"; // installs vscode mock before any other imports
import { bench, describe, beforeAll, afterAll } from "vitest";
import { targets, DUMMY_JCL, setupTargets } from "./setup";

beforeAll(() => setupTargets(), 60000);

describe("Jobs (JES)", () => {
    beforeAll(async () => {
        for (const target of targets) {
            try {
                target.job = await target.jes.submitJcl(DUMMY_JCL);
            } catch (e: any) {
                console.error(`[${target.name}] Submit job failed:`, e?.mDetails?.msg ?? e?.message);
            }
        }
    }, 60000);

    afterAll(async () => {
        for (const target of targets) {
            try {
                if (target.job) await target.jes?.deleteJob(target.job.jobname, target.job.jobid);
            } catch {}
        }
    });

    describe("Get job status", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                if (!target.job) throw new Error(`[${target.name}] No job available`);
                await target.jes.getJob(target.job.jobid);
            }, { throws: true });
        }
    });

    describe("List spool files", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                if (!target.job) throw new Error(`[${target.name}] No job available`);
                await target.jes.getSpoolFiles(target.job.jobname, target.job.jobid);
            }, { throws: true });
        }
    });

    describe("Get job JCL", () => {
        for (const target of targets) {
            bench(target.name, async () => {
                if (!target.job) throw new Error(`[${target.name}] No job available`);
                await target.jes.getJclForJob({ jobid: target.job.jobid, jobname: target.job.jobname });
            }, { throws: true });
        }
    });
});
