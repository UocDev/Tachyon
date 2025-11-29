import { execSync } from "child_process";

const token = process.env.BOT_TOKEN;

execSync(`
  git config user.name "queen-of-love-and-hate[bot]"
  git config user.email "queen-of-love-and-hate@users.noreply.github.com"
`);

try {
  execSync("git diff --quiet");
  console.log("No submodule changes.");
  process.exit(0);
} catch {}

execSync("git add .");

execSync(`
  git commit -m "chore: auto update submodules"
`);

execSync(`
  git push https://x-access-token:${token}@github.com/${process.env.GITHUB_REPOSITORY}.git HEAD
`);
