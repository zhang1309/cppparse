package file_send;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

public class Client {
	static void receive(DataInputStream dis,String address){
		String filename;
		long  fl;
		long now=0;
		int length;
		File file;
		File folder;
		byte[] receive;
		FileOutputStream fos=null;
		try {
			filename=dis.readUTF();
			fl=dis.readLong();
			if(address!=null){
				folder=new File(address);
				file=new File(address+"\\"+filename);
				}
			else {
			folder=new File("E:\\pc_fileReceive");
			file=new File("E:\\pc_fileReceive\\"+filename);
			}
			if(!folder.exists()){
				folder.mkdir();
			}
			
			
			if(!file.exists()){
				file.createNewFile();
			}
			System.out.print(fl);
			fos=new FileOutputStream(file);
			receive=new byte[65536];
			System.out.println("开始接收");
			while((length=dis.read(receive, 0, receive.length))>0){
				now+=length;
				fos.write(receive, 0, length);
				
			}
			fos.flush();
			System.out.println("接收完成");
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}finally{
			
				try {
					if(dis!=null){
						dis.close();
					}
					if(fos!=null)
						fos.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		
		
	}
	static void send(DataOutputStream dos,String address){
			File file;
			long fl;
			int length=0;
			long now=0;
			file=new File(address);
			FileInputStream fis = null;
			
			
			try {
				dos.writeUTF(file.getName());
				dos.writeLong(file.length());
				dos.flush();
				fis=new FileInputStream(file);
				byte[] send=new byte[65536];
				System.out.println("开始传送");
				while((length=fis.read(send, 0, send.length))>0){
					now+=length;
					dos.write(send,0,length);
				
				}
				System.out.print("传送完成");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}finally{
				
					try {
						if(dos!=null)
							dos.close();
						if(fis!=null)
							fis.close();                                                                                                                                                                                                                                                                                        
						
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
			}
	}
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Socket socket=null;
		try {
			//socket =new Socket("192.168.137.101",6463);
			socket =new Socket("10.170.69.197",7758);
			// socket =new Socket("127.0.0.1",6463);
			 //socket =new Socket("139.199.22.72",9527);
			DataInputStream dis=new DataInputStream(socket.getInputStream());
			DataOutputStream dos=new DataOutputStream(socket.getOutputStream());
			
			if(args[0].equals("r")){
				if(args.length==1)
					receive(dis,null);
				else receive(dis,args[1]);
			}
			else send(dos,args[1]);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}finally{
			if(socket!=null){
				try {
					socket.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}

}
