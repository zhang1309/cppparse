package file_send;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {
	static DataInputStream dis;
	static DataOutputStream dos;
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
			folder=new File(address);
			if(!folder.exists()){
				folder.mkdir();
			}
			
			file=new File(address+"\\"+filename);
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
		ServerSocket server=null;
		Socket socket=null;
		try {
			server=new ServerSocket(6463);
			socket=server.accept();
			
			dis=new DataInputStream(socket.getInputStream());
			dos=new DataOutputStream(socket.getOutputStream());
			
			if(args[0].equals("r")){
				receive(dis,args[1]);
			}
			else send(dos,args[1]);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}finally{
			if(server!=null)
				try {
					if(server!=null)
						server.close();
					if(socket!=null){
						socket.close();
					}
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			
		}
		
	}

}
