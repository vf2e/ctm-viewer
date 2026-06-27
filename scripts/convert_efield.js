const fs = require('fs');
const path = require('path');

const src = 'D:/2025WorkSpace/aimQ-Frontend/frontend/src/views/qmodel/common/b9076EField.js';
const out = path.resolve(__dirname, '../assets/data/b9076_efield.bin');

const text = fs.readFileSync(src, 'utf8');
const marker = 'export const b9076EField = ';
const start = text.indexOf(marker);
if (start < 0) {
  console.error('marker not found');
  process.exit(1);
}
const jsonText = text.slice(start + marker.length).trim().replace(/;\s*$/, '');
const arr = JSON.parse(jsonText);

const buf = Buffer.alloc(4 + arr.length * 16);
buf.writeUInt32LE(arr.length, 0);
let offset = 4;
for (const point of arr) {
  buf.writeFloatLE(point[0], offset);
  offset += 4;
  buf.writeFloatLE(point[1], offset);
  offset += 4;
  buf.writeFloatLE(point[2], offset);
  offset += 4;
  buf.writeFloatLE(point[3], offset);
  offset += 4;
}

fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, buf);
console.log(`Wrote ${arr.length} samples to ${out}`);
