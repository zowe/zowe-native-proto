import { existsSync, mkdirSync, readdirSync, readFileSync } from "fs";
import * as readline from "readline/promises";
import { resolve } from "path";
import { Client, SFTPWrapper } from "ssh2";

let config;

try {
  console.log(resolve(__dirname, `./../config.local.json`));
  config = JSON.parse(
    readFileSync(resolve(__dirname, `./../config.local.json`)).toString()
  );
} catch (e) {
  console.log(
    `You must create config.local.json (model from config.default.json) in same directory`
  );
}

const host = config.host;
const username = config.username;
let privateKey;
let password = config.password;
try {
  privateKey = readFileSync(config.privateKey);
} catch (e) {}

const localDeployDir = `./../../../lib`;        // from here
const deployDirectory = config.deployDirectory; // to here

const args = process.argv.slice(2);

const line = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

const connection = new Client();

connection.on(`ready`, async () => {
  switch (args[0]) {
    case `init`:
      await init(connection);
      break;
    case `deploy`:
      await deploy(connection);
      await convert(connection);
      break;
    case `deploy-build`:
      await deploy(connection);
      await convert(connection);
      await build(connection);
      break;
    case `get-listings`:
      await getListings(connection);
      break;
    case `get-dumps`:
      await getDumps(connection);
      break;
    case `clean`:
      await clean(connection);
      break;
    case `build`:
      await build(connection);
      break;
    default:
      console.log(`Unsupported command`);
      break;
  }

  line.close();
  connection.end();
});

connection.on(`close`, () => {
  console.log(`Client connection is closed`);
});

connection.on(`error`, () => {
  console.log(`Client connection is errored`);
});

if (!privateKey) {
  connection.connect({
    host,
    username,
    password,
  });
} else {
  connection.connect({
    host,
    username,
    privateKey,
  });
}

function getAllServerFiles() {
  let files: string[] = getServerFiles();

  // skip reading dirs if specific arguments were passed to upload
  if (args[1]) {
    return files;
  }

  const dirs = getDirs();

  dirs.forEach((dir) => {
    files.push(...getServerFiles(dir));
  });
  return files;
}

function getServerFiles(dir = ``) {
  if (args[1]) {
    return [...args.slice(1)];
  }

  const filesList: string[] = [];
  const files = readdirSync(resolve(__dirname, `${localDeployDir}/${dir}`), {
    withFileTypes: true,
  });

  files.forEach((file) => {
    if (file.isDirectory()) {
    } else {
      filesList.push(`${dir}${file.name}`);
    }
  });
  return filesList;
}

function getDirs(next = ``) {
  const dirs: string[] = [];

  const readDirs = readdirSync(
    resolve(__dirname, `${localDeployDir}/${next}`),
    { withFileTypes: true }
  );
  readDirs.forEach((dir) => {
    if (dir.isDirectory()) {
      const newDir = `${dir.name}/`;
      dirs.push(`${next}${newDir}`);
      dirs.push(...getDirs(`${next}${newDir}`));
    }
  });
  return dirs;
}

function parse_ds_view(raw: string) {
  console.log(`ds-view parsing`);
}

function parse_ds_ls_mem(raw: string) {
  console.log(`ds-ls-mem parsing`);
}

function parse_ds_ls(raw: string) {
  console.log(`ds-ls parsing`);
  console.log("Raw: ", raw);
  const lines = raw.trim().split(/\n/g);
  for (const line of lines) {
    const split = line.split(/\s+/g);
    console.log(
      JSON.stringify({
        name: split[0].trim(),
        dsorg: split[1].trim(),
      })
    );
  }
}

async function init(connection: Client) {
  const dirs = getDirs();

  return new Promise<void>((finish) => {
    console.log(`Making directories...`);
    const dirs = getDirs();

    connection.shell(false, (err, stream) => {
      if (err) {
        console.log(`Error: runCommand connection.exec error ${err}`);
        throw err;
      }

      stream.write(`mkdir -p ${deployDirectory}\n`);
      stream.write(`cd ${deployDirectory}\n`);
      for (let i = 0; i < dirs.length; i++) {
        console.log(`Creating ${dirs[i]}...`);
        stream.write(`mkdir -p ${dirs[i]}\n`);
      }
      stream.end(`exit\n`);

      stream.on(`close`, () => {
        console.log(`Directories created!`);
        finish();
      });
      stream.on(`data`, (part: Buffer) => {
        console.log(part.toString());
      });
      stream.stderr.on(`data`, (data: Buffer) => {
        console.log(data.toString());
      });
    });
  });
}

async function getListings(connection: Client) {

  if (args[1]) {
    await convert(connection, `IBM-1047`, `utf8`, [...args.slice(1)]);
    await retrieve(connection, [...args.slice(1)], `listings`);
    return
  }

  const resp = (
    await runCommandInShell(connection, `cd ${deployDirectory}\nls *.lst`)
  )
    .trim()
    .split(`\n`);

  await convert(connection, `IBM-1047`, `utf8`, resp);
  await retrieve(connection, resp, `listings`);
}


async function getDumps(connection: Client) {

  const resp = (
    await runCommandInShell(connection, `cd ${deployDirectory}\nls CEEDUMP.*`)
  )
    .trim()
    .split(`\n`);

  await convert(connection, `IBM-1047`, `utf8`, resp);
  await retrieve(connection, resp, `dumps`);
}

async function runCommandInShell(connection: Client, command: string) {
  return new Promise<string>((finish) => {
    let data: string = ``;
    connection.shell(false, (err, stream) => {
      if (err) {
        console.log(`Error: runCommand connection.exec error ${err}`);
        throw err;
      }
      stream.on(`close`, () => {
        finish(data);
      });
      stream.on(`data`, (part: Buffer) => {
        data += part.toString();
        // console.log(part.toString());
      });
      stream.stderr.on(`data`, (data: Buffer) => {
        console.log(data.toString());
      });
      stream.end(`${command}\nexit\n`);
    });
  });
}

async function retrieve(connection: Client, files: string[], targetDir: string) {
  return new Promise<void>((finish) => {
    console.log(`Retrieving files...`);

    connection.sftp(async (err, sftpcon) => {
      if (err) {
        console.log(`Retrieve err`);
        throw err;
      }

      for (let i = 0; i < files.length; i++) {
        if (!existsSync(`${targetDir}`)) mkdirSync(`${targetDir}`);
        const to = resolve(__dirname, `./../../${targetDir}/${files[i]}`);
        const from = `${deployDirectory}/${files[i]}`;
        // console.log(`from '${from}' to'${to}'`)
        await download(sftpcon, from, to);
      }
      console.log(`Get complete!`);
      finish();
    });
  });
}

async function deploy(connection: Client) {
  return new Promise<void>((finish) => {
    console.log(`Deploying files...`);
    const files = getAllServerFiles();

    connection.sftp(async (err, sftpcon) => {
      if (err) {
        console.log(`Deploy err`);
        throw err;
      }

      for (let i = 0; i < files.length; i++) {
        const from = resolve(__dirname, `${localDeployDir}/${files[i]}`);
        const to = `${deployDirectory}/${files[i]}`;
        await uploadFile(sftpcon, from, to);
      }
      console.log(`Deploy complete!`);
      finish();
    });
  });
}

async function convert(
  connection: Client,
  fromType = `utf8`,
  toType = `IBM-1047`,
  convFiles?: string[]
) {
  return new Promise<void>((finish) => {
    console.log(`Converting files from '${fromType}' to '${toType}'...`);
    const files = convFiles ?? getAllServerFiles();

    connection.shell(false, (err, stream) => {
      if (err) {
        console.log(`Error: runCommand connection.exec error ${err}`);
        throw err;
      }

      stream.write(`cd ${deployDirectory}\n`);
      for (let i = 0; i < files.length; i++) {
        stream.write(`mv ${files[i]} ${files[i]}.u\n`);
        stream.write(
          `iconv -f ${fromType} -t ${toType} ${files[i]}.u > ${files[i]}\n`
        );
        stream.write(`chtag -t -c ${toType} ${files[i]}\n`);
      }
      stream.end(`exit\n`);

      stream.on(`close`, () => {
        console.log(`Convert complete!`);
        finish();
      });
      stream.on(`data`, (part: Buffer) => {
        console.log(part.toString());
      });
      stream.stderr.on(`data`, (data: Buffer) => {
        console.log(data.toString());
      });
    });
  });
}

async function build(connection: Client) {
  console.log(`Building ...`);
  const resp = await runCommandInShell(
    connection,
    `cd ${deployDirectory} && make\n`
  );
  console.log(resp);
  console.log(`Build complete!`);
}

async function clean(connection: Client) {
  console.log(`Cleaning dir ...`);
  const resp = await runCommandInShell(
    connection,
    `cd ${deployDirectory} && make clean\n`
  );
  console.log(resp);
  console.log(`Clean complete`);
}

async function uploadFile(sftpcon: SFTPWrapper, from: string, to: string) {
  return new Promise<void>((finish) => {
    console.log(`Uploading '${from}' to ${to}`);
    sftpcon.fastPut(from, to, (err) => {
      if (err) {
        console.log(`Put err`);
        throw err;
      } else {
        finish();
      }
    });
  });
}

async function download(sftpcon: SFTPWrapper, from: string, to: string) {
  return new Promise<void>((finish) => {
    console.log(`Downloading '${from}' to ${to}`);
    sftpcon.fastGet(from, to, (err) => {
      if (err) {
        console.log(`Get err`);
        throw err;
      } else {
        finish();
      }
    });
  });
}
