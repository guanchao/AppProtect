package com.dexencrypt;

import java.io.InputStream;
import java.lang.reflect.Method;
import java.security.MessageDigest;
import java.util.zip.Adler32;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

public class Security {
	private Context context;

	public Security(Context context){
		this.context = context;
	}
	
	public String invokeHiddenMethod(){
		int dexlength = 616;
		AssetManager assetManager = this.context.getAssets();
		try {
			InputStream encryptedStream = assetManager.open("Test.modified.encrypted.dex");
			byte[]dexFile = DESUtils.decryptDexFile(encryptedStream, dexlength);
			
			//Step1:修改方法A的DexMethod，方法A的DexMethod的偏移地址為：0x1d4
			int offset = 0x1d4;
			dexFile[offset++]=1; //methodIdx
			dexFile[offset++]=1; //accessFlags
			dexFile[offset++]=(byte)0x9c; //codeOff
			dexFile[offset++]=(byte)0x02; //codeOff
			
			//Step2:重新計算signature(SHA1)
			calcSignature(dexFile);
			
			//Step3:重新計算checksum
			calcChecksum(dexFile);
			
			//Step4:通過反射調用隱藏的方法methodA
			Class dexFileClass = context.getClassLoader().loadClass("dalvik.system.DexFile");
			Method[] arrayOfMethod = Class.forName("dalvik.system.DexFile").getDeclaredMethods();
			Method openDexFileMethod = null;
			Method defineCLassMethod = null;
			int cookie = 0;
			
			for(int i = 0; i < arrayOfMethod.length; i++){
				//private native static int openDexFile(byte[] fileContents)
				if(arrayOfMethod[i].getName().equalsIgnoreCase("openDexFile") && arrayOfMethod[i].getParameterTypes().length == 1){
					openDexFileMethod = arrayOfMethod[i];
					openDexFileMethod.setAccessible(true);
				}
				
				// private native static Class defineClass(String name, ClassLoader loader, int cookie)
				if(arrayOfMethod[i].getName().equalsIgnoreCase("defineClass") && arrayOfMethod[i].getParameterTypes().length == 3){
					defineCLassMethod = arrayOfMethod[i];
					defineCLassMethod.setAccessible(true);
				}
			}
			
			Object[] arrayOfObject = new Object[1];
			arrayOfObject[0] = dexFile;
			
			if(openDexFileMethod != null){
				cookie = ((Integer) openDexFileMethod.invoke(dexFileClass, arrayOfObject)).intValue();
			}
			
			Object[] params = new Object[3];
			params[0] = "Test";
		    params[1] = dexFileClass.getClassLoader();
		    params[2] = Integer.valueOf(cookie);
		    
		    Class clazz = null;
		    if(defineCLassMethod != null){
		    	clazz = (Class) defineCLassMethod.invoke(dexFileClass, params);
		    }
		    Method methodA = null;
		    Method[] allMethods = clazz.getDeclaredMethods();
		    
		    for(int i = 0; i<allMethods.length; i++){
		    	if(allMethods[i].getName().equalsIgnoreCase("methodA")){
		    		methodA = allMethods[i];
		    	}
		    }
		    
		    if(methodA != null){
		    	Object obj = clazz.getDeclaredConstructor(null).newInstance();
		    	return (String) methodA.invoke(obj, null);
		    }
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		
	}
	
	 private static void calcSignature(byte dexFile[]){
		try {
			MessageDigest digest;
			digest = MessageDigest.getInstance("SHA-1");
			digest.reset();
			digest.update(dexFile, 32, dexFile.length-32);
			digest.digest(dexFile, 12, 20);
		} catch (Exception e) {
			e.printStackTrace();
		} 
	}
	 
	 private static void calcChecksum(byte dexFile[]){
	        Adler32 a32 = new Adler32();
	        a32.update(dexFile, 12, dexFile.length - 12);
	        int sum = (int)a32.getValue();
	        dexFile[8] = (byte)sum;
	        dexFile[9] = (byte)(sum >> 8);
	        dexFile[10] = (byte)(sum >> 16);
	        dexFile[11] = (byte)(sum >> 24);
	    }

}
