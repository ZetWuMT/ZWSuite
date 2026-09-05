import asyncio
import sys

import edge_tts


def main() -> int:
    text_path, voice, media_path = sys.argv[1], sys.argv[2], sys.argv[3]
    with open(text_path, encoding="utf-8") as text_file:
        text = text_file.read()
    asyncio.run(edge_tts.Communicate(text, voice=voice).save(media_path))
    return 0


if __name__ == "__main__":
    sys.exit(main())
