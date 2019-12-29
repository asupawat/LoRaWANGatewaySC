#ifndef __AESM__
#define __AESM__

#include <AES.h>
#include <base64.hpp>

// CBC implementation
class AESM {
	public:
	AES aes;
	byte key[32];
	//String keystr = "";

	AESM(byte* key) {
		memcpy(this->key, key, 32);
		// randomSeed(analogRead(0)); // RANDOM_REG32 ??
/*
		unsigned int b64Length = (4 * (32 + 2) / 3) + 1;
		unsigned char base64[b64Length];
		unsigned int base64_length = encode_base64(this->key, 32, base64);
		this->keystr = String((char*)base64);
*/
	}

	static uint16_t calculateCipherLength(int plainLength) {
		unsigned int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
		unsigned int cipherLength = N_BLOCK + padedLength;
		return cipherLength;
	}

	static uint16_t calculatePlainLength(int cipherLength) {
		unsigned int plainLength = cipherLength - N_BLOCK;
		return plainLength;
	}

	static void getRidOfPadding(byte* plain, unsigned int plainLength) {
		byte last = plain[plainLength - 1];
		// get rid of pkcs7 padding
		if (0 < last && last <= 16)
			for (byte i = plainLength - 1; i >= (plainLength - last); i--)
				plain[i] = '\0';
	}

	void encrypt(byte* plain, unsigned int plainLength, byte* cipher, unsigned int cipherLength) {
		byte iv[N_BLOCK];
		for (unsigned int i = 0; i < N_BLOCK; i++) iv[i] = random(256);
		memcpy(cipher, iv, N_BLOCK);
		this->aes.do_aes_encrypt(plain, plainLength, cipher + N_BLOCK, this->key, 256, iv);
	}

	void decrypt(byte* plain, byte* cipher, unsigned int cipherLength) {
		byte iv[N_BLOCK];
		memcpy(iv, cipher, N_BLOCK);
		this->aes.do_aes_decrypt(cipher + N_BLOCK, cipherLength, plain, this->key, 256, iv);
	}

	String encrypt(byte* plain, int plainLength) {
		int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
		unsigned int length = N_BLOCK + padedLength;
		byte cipher[length];
		this->encrypt(plain, plainLength, cipher, length);
		unsigned int b64Length = (4 * (length + 2) / 3) + 2;
		unsigned char base64[b64Length]; base64[b64Length - 1] = '\0';
		unsigned int base64_length = encode_base64(cipher, length, base64);
		return String((char*)base64);
	}

	String decrypt(byte* base64) {
		unsigned int binary_length = decode_base64_length(base64);
		// this is because in this implementation we assume that the first 16 bytes of the message
		// are the encryption iv followed by the encrypted data
		if (2 * N_BLOCK <= binary_length) {
			uint16_t cipherLength = binary_length - N_BLOCK;
			unsigned char cipher[binary_length];
			binary_length = decode_base64(base64, cipher);
			byte plain[cipherLength + 1];
			plain[cipherLength] = '\0';
			this->decrypt(plain, cipher, cipherLength);
			byte last = plain[cipherLength - 1];
			// get rid of pkcs7 padding
			if (0 < last && last <= N_BLOCK)
				for (uint16_t i = cipherLength - 1; i >= (cipherLength - last); i--)
					plain[i] = '\0';

			return String((char*) plain);
		} else {
			return "";
		}
	}
};

#endif