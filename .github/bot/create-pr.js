const https = require("https");

const token = process.env.BOT_TOKEN;
const repo = process.env.GITHUB_REPOSITORY; // contoh: UocDev/Tachyon
const base = "master";                      // branch tujuan PR
const head = "DonQuixote";                  // branch hasil update submodule

function api(path, method = "GET", body = null) {
  return new Promise((resolve, reject) => {
    const data = body ? JSON.stringify(body) : null;

    const req = https.request(
      {
        hostname: "api.github.com",
        path,
        method,
        headers: {
          "User-Agent": "submodule-bot",
          Authorization: `token ${token}`,
          Accept: "application/vnd.github+json",
          ...(data && {
            "Content-Type": "application/json",
            "Content-Length": Buffer.byteLength(data)
          })
        }
      },
      res => {
        let raw = "";
        res.on("data", d => (raw += d));
        res.on("end", () => {
          try {
            resolve(JSON.parse(raw));
          } catch {
            resolve(raw);
          }
        });
      }
    );

    req.on("error", reject);
    if (data) req.write(data);
    req.end();
  });
}

(async () => {
  // 1. Cek apakah PR dari DonQuixote → master sudah ada
  const prs = await api(`/repos/${repo}/pulls?head=${repo.split("/")[0]}:${head}&base=${base}`);

  if (Array.isArray(prs) && prs.length > 0) {
    console.log("Pull Request already exists:", prs[0].html_url);
    return;
  }

  // 2. Buat PR baru
  const pr = await api(
    `/repos/${repo}/pulls`,
    "POST",
    {
      title: "chore: auto update submodules",
      head,
      base,
      body: "Automated submodule update by bot via GitHub Actions."
    }
  );

  console.log("Pull Request created:", pr.html_url);
})().catch(err => {
  console.error("Failed to create PR:", err);
  process.exit(1);
});
