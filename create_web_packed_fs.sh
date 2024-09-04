cmake -S pack_src -B pack_src/build

cd web_src
npm install
npm run build

cd ../

pack_src/build/pack -s web_src/ web_src/dist/* web_src/dist/assets/* > packed_fs.c 