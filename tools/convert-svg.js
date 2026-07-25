const sharp = require('sharp');
const fs = require('fs');

const sizes = [
  { name: 'round-aplite', width: 144, height: 168, fit: 'contain' },
  { name: 'round-color', width: 144, height: 168, fit: 'contain' },
  { name: 'round-chalk', width: 180, height: 180, fit: 'contain' },
  { name: 'round-diorite', width: 144, height: 168, fit: 'contain' },
  { name: 'round-emery', width: 200, height: 228, fit: 'contain' },
  { name: 'round-flint', width: 144, height: 168, fit: 'contain' },
  { name: 'round-gabbro', width: 260, height: 260, fit: 'contain' },
];

const blackSizes = [
  { name: 'black-aplite', width: 144, height: 168, fit: 'contain' },
  { name: 'black-color', width: 144, height: 168, fit: 'contain' },
  { name: 'black-chalk', width: 180, height: 180, fit: 'contain' },
  { name: 'black-diorite', width: 144, height: 168, fit: 'contain' },
  { name: 'black-emery', width: 200, height: 228, fit: 'contain' },
  { name: 'black-flint', width: 144, height: 168, fit: 'contain' },
  { name: 'black-gabbro', width: 260, height: 260, fit: 'contain' },
];

async function convert() {
  // White round face
  const svgBuffer = fs.readFileSync('sbb-face.svg');
  for (const size of sizes) {
    await sharp(svgBuffer)
      .resize(size.width, size.height, { fit: size.fit, background: { r: 255, g: 255, b: 255, alpha: 0 } })
      .png()
      .toFile(`../resources/images/sbb-face-${size.name}.png`);
    console.log(`Created sbb-face-${size.name}.png`);
  }

  // Black round face
  const blackSvgBuffer = fs.readFileSync('sbb-face-black.svg');
  for (const size of blackSizes) {
    await sharp(blackSvgBuffer)
      .resize(size.width, size.height, { fit: size.fit, background: { r: 0, g: 0, b: 0, alpha: 0 } })
      .png()
      .toFile(`../resources/images/sbb-face-${size.name}.png`);
    console.log(`Created sbb-face-${size.name}.png`);
  }

  console.log('Conversion complete!');
}

convert().catch(console.error);
