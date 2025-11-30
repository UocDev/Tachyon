const { execSync } = require("child_process");

const token = process.env.BOT_TOKEN;
const repo = process.env.GITHUB_REPOSITORY;
const branch = "DonQuixote";

execSync(`
  git config user.name "queen-of-love-and-hate[bot]"
  git config user.email "queen-of-love-and-hate@users.noreply.github.com"
`);

execSync(`git checkout -B ${branch}`);

try {
  execSync("git diff --quiet");
  console.log("No submodule changes.");
  process.exit(0);
} catch {}

execSync("git add .");
execSync(`git commit -m "chore: auto update submodules"`);

execSync(`
  git push https://x-access-token:${token}@github.com/${repo}.git ${branch} --force
`);
