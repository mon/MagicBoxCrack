import xxtea # pip install xxtea
import os

key_enc = b'\xb9U\x1eS\x00\x8a\xda>Q:IV\x18%\xa1['
key_key = b'\t\r\x02\x05#!\x1e!\x1a\x10\x0bW?XJ\x03'

key = xxtea.decrypt(key_enc, key_key, padding=False)

encrypt_folder = 'D:/Rhythm/MagicBoxVer4-9'
decrypt_folder = 'D:/Rhythm/MagicBoxVer4-9_uncrypto'

def decrypt(path, source_dir, dest_dir, key):
    print(path)
    source_path = os.path.join(source_dir, path)
    dest_path = os.path.join(dest_dir, path)
    dest_folder = os.path.dirname(dest_path)
    os.makedirs(dest_folder, exist_ok=True)

    path, ext = os.path.splitext(dest_path)

    with open(source_path, 'rb') as f:
        enc = f.read()

    dec = xxtea.decrypt(enc, key, padding=False)
    # literally only 2 types of file in this game
    if dec.startswith(b'\x89PNG\r\n\x1a\n'):
        ext = '.png'
    else:
        ext = '.txt'

    with open(path + ext, 'wb') as f:
        f.write(dec)

# traverse root directory, and list directories as dirs and files as files
for root, dirs, files in os.walk(encrypt_folder):
    root = os.path.relpath(root, encrypt_folder)
    for file in files:
       path = os.path.join(root, file)
       if file.lower().endswith('.ltm'):
           decrypt(path, encrypt_folder, decrypt_folder, key)
