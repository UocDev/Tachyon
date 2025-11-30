const fetch = require("node-fetch");
const jwt = require("jsonwebtoken");

const APP_ID = process.env.APP_ID;
const INSTALL_ID = process.env.INSTALLATION_ID;
const PRIVATE_KEY = process.env.APP_PRIVATE_KEY
  ? process.env.APP_PRIVATE_KEY.replace(/\\n/g, "\n")
  : null;

if (!APP_ID || !INSTALL_ID || !PRIVATE_KEY) {
  console.error("Missing APP_ID, INSTALLATION_ID, or APP_PRIVATE_KEY");
  process.exit(1);
}

const jwtToken = jwt.sign(
  { iss: APP_ID },
  PRIVATE_KEY,
  { algorithm: "RS256", expiresIn: "10m" }
);

(async () => {
  const res = await fetch(
    `https://api.github.com/app/installations/${INSTALL_ID}/access_tokens`,
    {
      method: "POST",
      headers: {
        Authorization: `Bearer ${jwtToken}`,
        Accept: "application/vnd.github+json",
        "User-Agent": "github-app-bot",
      }
    }
  );

  if (!res.ok) {
    const text = await res.text();
    console.error("GitHub API Error:", text);
    process.exit(1);
  }

  const data = await res.json();

  if (!data.token) {
    console.error("Installation token not returned");
    process.exit(1);
  }

  // mask secret in logs
  console.log(`::add-mask::${data.token}`);

  // export as workflow output
  console.log(`BOT_TOKEN=${data.token}`);
})();
