import fetch from "node-fetch";

const token = process.env.BOT_TOKEN;
const repo = process.env.GITHUB_REPOSITORY;

const [owner, name] = repo.split("/");

const res = await fetch(`https://api.github.com/repos/${owner}/${name}/pulls`, {
  method: "POST",
  headers: {
    Authorization: `token ${token}`,
    Accept: "application/vnd.github+json"
  },
  body: JSON.stringify({
    title: "chore: auto update submodules",
    head: "DonQuixote",
    base: "master",
    body: "Automated daily submodule update via GitHub App bot."
  })
});

const data = await res.json();

if (!res.ok) {
  console.error("Failed to create PR:", data);
  process.exit(1);
}

console.log("Pull Request created:", data.html_url);
