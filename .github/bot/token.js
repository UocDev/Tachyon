import jwt from "jsonwebtoken";
import fetch from "node-fetch";

const APP_ID = process.env.APP_ID;
const INSTALL_ID = process.env.INSTALLATION_ID;
const PRIVATE_KEY = process.env.APP_PRIVATE_KEY.replace(/\\n/g, "\n");

const jwtToken = jwt.sign(
  { iss: APP_ID },
  PRIVATE_KEY,
  { algorithm: "RS256", expiresIn: "10m" }
);

const res = await fetch(
  `https://api.github.com/app/installations/${INSTALL_ID}/access_tokens`,
  {
    method: "POST",
    headers: {
      Authorization: `Bearer ${jwtToken}`,
      Accept: "application/vnd.github+json"
    }
  }
);

const data = await res.json();

console.log(`::add-mask::${data.token}`);
console.log(`BOT_TOKEN=${data.token}`);
