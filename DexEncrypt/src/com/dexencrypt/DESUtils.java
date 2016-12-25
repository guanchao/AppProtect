package com.dexencrypt;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.Key;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

public class DESUtils {

	public static final String KEY_ALGORITHM = "DES";
	public static final String CIPHER_ALGORITHM = "DES/ECB/PKCS5Padding";
	
	public static void main(String[] args) throws Exception {
		String inputStr = "DES";
		byte[] inputData = inputStr.getBytes();
		System.err.println("原文:\t" + inputStr);
		
		//初始化密钥
		byte[] key = initKey();
		System.err.println("加密后:\t" + Common.byteArrayToHex(key));
		
		//加密
		inputData = encrypt(inputData, key);
		System.err.println("加密后:\t" + Common.byteArrayToHex(inputData));
		
		//解密
		byte[] outputData = decrypt(inputData, key);
		String outputStr = new String(outputData);
		System.err.println("解密后:\t" + outputStr);
		
	}
	
	public static byte[] decryptDexFile(InputStream input, int dexLength) {
		try {
			byte[] encryptedData = new byte[dexLength];
			input.read(encryptedData);
			input.close();
			
			//解密dex文件字节流
			byte[] key = "26798CE99E68B6E9".getBytes();
			byte[] decryptedData = decrypt(encryptedData, key);
			
			return decryptedData;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 转换密钥
	 * @param key
	 * @return
	 * @throws Exception
	 */
	private static Key toKey(byte[] key) throws Exception{
		DESKeySpec dks = new DESKeySpec(key);
		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance(KEY_ALGORITHM);
		SecretKey secretKey = keyFactory.generateSecret(dks);
		
		return secretKey;
	}
	
	/**
	 * 待解密数据
	 * @param data
	 * @param key
	 * @return
	 * @throws Exception
	 */
	public static byte[] decrypt(byte[] data, byte[]key) throws Exception{
		Key k = toKey(key);
		Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);
		cipher.init(Cipher.DECRYPT_MODE, k);
		return cipher.doFinal(data);
	}
	
	/**
	 * 待加密数据
	 * @param data
	 * @param key
	 * @return
	 * @throws Exception
	 */
	public static byte[]encrypt(byte[] data, byte[] key) throws Exception{
		Key k = toKey(key);
		Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);
		cipher.init(Cipher.ENCRYPT_MODE, k);
		return cipher.doFinal(data);
	}
	
	/**
	 * 生成密钥
	 * @return
	 * @throws Exception
	 */
	public static byte[]initKey() throws Exception{
		KeyGenerator kg = KeyGenerator.getInstance(KEY_ALGORITHM);
		kg.init(56);
		SecretKey secretKey = kg.generateKey();
		return secretKey.getEncoded();
	}
}