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

import {
  FileManagement,
  Gui,
  imperative,
  ZoweVsCodeExtension,
} from "@zowe/zowe-explorer-api";
import * as fs from "fs";
import * as vscode from "vscode";
import { ZClientUtils, ZSshClient } from "zowe-native-proto-sdk";
import { homedir } from "os";
import * as path from "node:path";
import { ProfileConstants } from "@zowe/core-for-zowe-sdk";
import { ZosUssProfile } from "@zowe/zos-uss-for-zowe-sdk";
import { sshConfigExt } from "zowe-native-proto-sdk";
import { Client } from "ssh2";

// biome-ignore lint/complexity/noStaticOnlyClass: Utilities class has static methods
export class SshConfigUtils {
  public static getServerPath(hostname: string): string {
    const serverPathMap = vscode.workspace
      .getConfiguration("zowe-native-proto-vsce")
      .get<Record<string, string>>("serverPath");
    return serverPathMap?.[hostname] ?? ZSshClient.DEFAULT_SERVER_PATH;
  }

  public static async promptForProfile(
    profileName?: string
  ): Promise<imperative.IProfileLoaded | undefined> {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    const profCache = zoweExplorerApi
      .getExplorerExtenderApi()
      .getProfilesCache();
    const profInfo = await profCache.getProfileInfo();
    const configExists = profInfo.getTeamConfig().exists;

    if (profileName) {
      return profCache.getLoadedProfConfig(profileName, "ssh");
    }

    const sshProfiles = (await profCache.fetchAllProfilesByType("ssh")).filter(
      ({ name, profile }) => name && profile?.host
    );

    const migratedConfigs = (await ZClientUtils.migrateSshConfig()).filter(
      (migratedConfig) =>
        !sshProfiles.some(
          (sshProfile) => sshProfile.profile?.host === migratedConfig.hostname
        )
    );

    const qpItems: vscode.QuickPickItem[] = [
      { label: "$(plus) Add New SSH Host..." },
      ...sshProfiles.map(({ name, profile }) => ({
        label: name!,
        description: profile?.host,
      })),
      {
        label: "Migrate From SSH Config",
        kind: vscode.QuickPickItemKind.Separator,
      },
      ...migratedConfigs.map(({ name, hostname }) => ({
        label: name!,
        description: hostname,
      })),
    ];

    const result = await Gui.showQuickPick(qpItems, {
      title: "Choose an SSH host",
    });

    if (!result) return;

    if (
      result === qpItems[0] ||
      migratedConfigs.some(({ name }) => result.label === name)
    ) {
      const isNewConfig = result === qpItems[0];
      let selectedProfile = isNewConfig
        ? await SshConfigUtils.createNewConfig()
        : migratedConfigs.find(({ name }) => result.label === name);

      if (!selectedProfile) return;

      if (!isNewConfig && !configExists)
        await SshConfigUtils.createZoweSchema();

      selectedProfile = await SshConfigUtils.getNewProfileName(selectedProfile);
      if (!selectedProfile?.name) {
        vscode.window.showWarningMessage("SSH setup cancelled.");
        return;
      }

      let validationResult = await SshConfigUtils.validateConfig(
        selectedProfile
      );
      if (typeof validationResult === "string")
        selectedProfile.keyPassphrase = validationResult;

      if (!validationResult) {
        selectedProfile = await SshConfigUtils.promptForAuth(selectedProfile);
        if (!selectedProfile) return;
      }

      if (!selectedProfile.privateKey && !selectedProfile.password) {
        vscode.window.showWarningMessage("SSH setup cancelled.");
        return;
      }

      await SshConfigUtils.setProfile(selectedProfile);
      return {
        name: selectedProfile.name,
        message: "",
        failNotFound: false,
        type: "ssh",
        profile: {
          host: selectedProfile.hostname,
          name: selectedProfile.name,
          password: selectedProfile.password,
          user: selectedProfile.user,
          privateKey: selectedProfile.privateKey,
          handshakeTimeout: selectedProfile.handshakeTimeout,
          port: selectedProfile.port,
          keyPassphrase: selectedProfile.keyPassphrase,
        },
      };
    }

    return sshProfiles.find(({ name }) => name === result.label);
  }

  public static showSessionInTree(profileName: string, visible: boolean): void {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    for (const setting of [
      "zowe.ds.history",
      "zowe.uss.history",
      "zowe.jobs.history",
    ]) {
      const localStorage = (
        zoweExplorerApi.getExplorerExtenderApi() as any
      ).getLocalStorage();
      const treeHistory = localStorage.getValue(setting);
      treeHistory.sessions = treeHistory.sessions.filter(
        (session: any) => session !== profileName
      );
      if (visible) {
        treeHistory.sessions.push(profileName);
      }
      localStorage.setValue(setting, treeHistory);
    }
    zoweExplorerApi.getExplorerExtenderApi().reloadProfiles("ssh");
  }

  private static async createNewConfig(): Promise<sshConfigExt | undefined> {
    let SshProfile: sshConfigExt = {};
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    const profInfo = await zoweExplorerApi
      .getExplorerExtenderApi()
      .getProfilesCache()
      .getProfileInfo();
    const configExists = profInfo.getTeamConfig().exists;

    if (!configExists) await SshConfigUtils.createZoweSchema();

    const sshResponse = await vscode.window.showInputBox({
      prompt: "Enter SSH connection command",
      placeHolder: "E.g. ssh user@example.com",
    });
    if (sshResponse === undefined) {
      vscode.window.showWarningMessage("SSH setup cancelled.");
      return undefined;
    }
    const sshRegex = /^ssh\s+([a-zA-Z0-9_-]+)@([a-zA-Z0-9.-]+)/;
    const flagRegex = /--(\w+)(?:\s+("[^"]+"|'[^']+'|\S+))?/g;

    const sshMatch = sshResponse.match(sshRegex);
    if (!sshMatch) {
      vscode.window.showErrorMessage(
        "Invalid SSH command format. Ensure it matches the expected pattern."
      );
      return undefined;
    }
    SshProfile.user = sshMatch[1];
    SshProfile.hostname = sshMatch[2];

    let flagMatch;
    while ((flagMatch = flagRegex.exec(sshResponse)) !== null) {
      const [, flag, value] = flagMatch;
      const normalizedFlag = flag.toLowerCase();

      // Check for missing value
      if (!value) {
        vscode.window.showErrorMessage(`Missing value for flag --${flag}.`);
        return undefined;
      }

      const unquotedValue = value.replace(/^["']|["']$/g, ""); // Remove surrounding quotes

      // Map aliases to consistent keys
      if (normalizedFlag === "p" || normalizedFlag === "port") {
        const portNumber = Number.parseInt(unquotedValue, 10);
        if (Number.isNaN(portNumber)) {
          vscode.window.showErrorMessage(
            `Invalid value for flag --${flag}. Port must be a valid number.`
          );
          return undefined;
        }
        SshProfile.port = portNumber;
      } else if (normalizedFlag === "pk" || normalizedFlag === "privateKey") {
        SshProfile.privateKey = unquotedValue;
      }

      // Validate if quotes are required
      if (/\s/.test(unquotedValue) && !/^["'].*["']$/.test(value)) {
        vscode.window.showErrorMessage(
          `Invalid value for flag --${flag}. Values with spaces must be quoted.`
        );
        return undefined;
      }
    }

    return SshProfile;
  }

  private static async promptForAuth(
    selectedConfig: sshConfigExt | undefined
  ): Promise<sshConfigExt | undefined> {
    let qpItems: vscode.QuickPickItem[] = [
      { label: "$(lock) Password" },
      { label: "$(key) Private Key" },
      { label: "$(key) Private Key with Passphrase" },
    ];

    const selectedOption = await vscode.window.showQuickPick(qpItems, {
      title: "Select an authentication method",
      ignoreFocusOut: true,
    });

    // Priority for default Private Key URI
    // 1. IdentityFile/PrivateKey if passed from config
    // 2. .ssh directory within homedir()
    // 3. homedir()

    let defaultPrivateKeyUri = vscode.Uri.file(
      selectedConfig?.privateKey
        ? selectedConfig.privateKey
        : fs.existsSync(path.join(homedir(), ".ssh"))
        ? path.join(homedir(), ".ssh")
        : homedir()
    );

    let privateKeyUri;
    if (selectedOption) {
      switch (selectedOption.label) {
        case "$(lock) Password":
          selectedConfig!.password = await vscode.window.showInputBox({
            title: "Enter Password",
            password: true,
            placeHolder: "Enter your password",
            ignoreFocusOut: true,
          });
          break;

        case "$(key) Private Key":
          privateKeyUri = await vscode.window.showOpenDialog({
            canSelectFiles: true,
            canSelectMany: false,
            title: "Select Private Key File",
            defaultUri: defaultPrivateKeyUri,
          });
          if (privateKeyUri && privateKeyUri.length > 0) {
            selectedConfig!.privateKey = privateKeyUri[0].fsPath;
          }
          break;

        case "$(key) Private Key with Passphrase":
          privateKeyUri = await vscode.window.showOpenDialog({
            canSelectFiles: true,
            canSelectMany: false,
            title: "Select Private Key File",
            defaultUri: defaultPrivateKeyUri,
          });
          if (privateKeyUri && privateKeyUri.length > 0) {
            selectedConfig!.privateKey = privateKeyUri[0].fsPath;
            selectedConfig!.keyPassphrase = await vscode.window.showInputBox({
              title: "Enter Passphrase",
              password: true,
              placeHolder: "Enter passphrase for private key",
              ignoreFocusOut: true,
            });
          }
          break;
        default:
          break;
      }
      return selectedConfig;
    }
  }

  private static async getNewProfileName(
    selectedProfile: sshConfigExt
  ): Promise<sshConfigExt | undefined> {
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    const profCache = zoweExplorerApi
      .getExplorerExtenderApi()
      .getProfilesCache();
    const profiles = await profCache.fetchAllProfiles();
    let isUniqueName = false;

    if (!selectedProfile.name) selectedProfile.name = selectedProfile.hostname;
    while (!isUniqueName) {
      selectedProfile.name = await vscode.window.showInputBox({
        prompt: "Enter a name for the SSH config",
        value: selectedProfile.name!.replace(/\./g, "_"),
        validateInput: (input) =>
          input.includes(".") ? "Name cannot contain '.'" : null,
      });
      const existingProfile = profiles.find(
        (profile) => profile.name === selectedProfile.name
      );
      if (existingProfile) {
        const overwriteResponse = await vscode.window.showQuickPick(
          ["Yes", "No"],
          {
            placeHolder: `A profile with the name "${selectedProfile.name}" already exists. Do you want to overwrite it?`,
          }
        );
        if (overwriteResponse === "Yes") {
          isUniqueName = true;
        } else {
        }
      } else {
        isUniqueName = true;
      }
    }
    return selectedProfile;
  }

  private static async setProfile(
    selectedConfig: sshConfigExt | undefined
  ): Promise<void> {
    //Profile information
    const zoweExplorerApi = ZoweVsCodeExtension.getZoweExplorerApi();
    const profCache = zoweExplorerApi
      .getExplorerExtenderApi()
      .getProfilesCache();
    const profInfo = await profCache.getProfileInfo();
    const configApi = profInfo.getTeamConfig().api;
    // Create the base config object
    const config = {
      type: "ssh",
      properties: {
        user: selectedConfig?.user,
        host: selectedConfig?.hostname,
        privateKey: selectedConfig?.privateKey,
        port: selectedConfig?.port || 22,
        keyPassphrase: selectedConfig?.keyPassphrase,
        password: selectedConfig?.password,
      },
      secure: [],
    };
    //if password or KP is defined, make them secure
    if (selectedConfig?.password) config.secure.push("password" as never);
    if (selectedConfig?.keyPassphrase)
      config.secure.push("keyPassphrase" as never);

    configApi.profiles.set(selectedConfig?.name!, config);
    await profInfo.getTeamConfig().save();
  }

  // Cloned method
  public static async createZoweSchema(): Promise<void> {
    try {
      let user = false;
      let global = true;
      let rootPath = FileManagement.getZoweDir();
      const workspaceDir = ZoweVsCodeExtension.workspaceRoot;

      const config = await imperative.Config.load("zowe", {
        homeDir: FileManagement.getZoweDir(),
        projectDir: FileManagement.getFullPath(rootPath),
      });
      if (workspaceDir != null) {
        config.api.layers.activate(user, global, rootPath);
      }

      const knownCliConfig: imperative.ICommandProfileTypeConfiguration[] = [
        ZosUssProfile,
        ProfileConstants.BaseProfile,
      ];
      config.setSchema(imperative.ConfigSchema.buildSchema(knownCliConfig));

      // Note: IConfigBuilderOpts not exported
      // const opts: IConfigBuilderOpts = {
      const opts: any = {
        // getSecureValue: this.promptForProp.bind(this),
        populateProperties: true,
      };
      // Build new config and merge with existing layer
      const impConfig: Partial<imperative.IImperativeConfig> = {
        profiles: knownCliConfig,
        baseProfile: ProfileConstants.BaseProfile,
      };
      const newConfig: imperative.IConfig =
        await imperative.ConfigBuilder.build(impConfig, global, opts);

      config.api.layers.merge(newConfig);
      await config.save(false);
    } catch (err) {}
  }
  private static async validateConfig(
    newConfig: sshConfigExt
  ): Promise<boolean | string> {
    const attemptConnection = async (config: sshConfigExt) => {
      return new Promise((resolve, reject) => {
        const sshClient = new Client();
        const testConnection = { ...config } as any; // Create a shallow copy

        // Parse privateKey
        if (
          testConnection.privateKey &&
          typeof testConnection.privateKey === "string"
        ) {
          testConnection.privateKey = fs.readFileSync(
            path.normalize(testConnection.privateKey),
            "utf8"
          );
        }

        // Test credentials
        sshClient
          .connect(testConnection)
          .on("error", (err) => {
            if (err) {
              reject(err);
            }
          })
          .on("ready", () => {
            resolve(null);
          });
      });
    };
    try {
      await attemptConnection(newConfig);
    } catch (err) {
      // If auth fails because of no passphrase, prompt for one
      if ((err as any).message.includes("but no passphrase given")) {
        let reservePassphrase = newConfig!.keyPassphrase;
        (newConfig as any).passphrase = await vscode.window.showInputBox({
          title: "Enter Passphrase",
          password: true,
          placeHolder: "Enter passphrase for private key",
          ignoreFocusOut: true,
        });
        try {
          // Auth with passphrase
          await attemptConnection(newConfig);
          return (newConfig as any).passphrase;
        } catch {
          // Auth with passphrase still failed
          vscode.window.showErrorMessage("Passphrase Authentication Failed");
          // Start with a clean slate if auth fails
          delete (newConfig as any).passphrase;
          delete (newConfig as any).privateKey;
          return false;
        }
      } else {
        return false;
      }
    }
    return true;
  }
}
