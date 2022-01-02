import java.io.*;
import java.net.*;

public class NetworkBridge{
	static boolean out=true;
	public static void main(String[] args) throws IOException{
		if(args.length!=1&&args.length!=2){
			System.out.println("Need 1 argument");
			System.exit(-1);
		}
		if(args.length==2){
			if(args[1].equals("false"))out=false;
			else if(args[1].equals("true"))out=true;
			else{
				System.out.println("true/false");
				System.exit(-3);
			}
		}
		int port=0;
		try{
			port=Integer.parseInt(args[0]);
			if(port<0&&port>65535)throw new Exception();
		}catch(Exception e){
			System.out.println("Not a port");
			System.exit(-2);
		}
		ServerSocket ss=new ServerSocket(10000);
		System.out.println("Built server,port 10000");
		while(true){
			try{
				final Socket client=ss.accept();
				System.out.println("Someone comes...");
				final Socket server=new Socket("127.0.0.1",port);
				new Thread(){
					public void run(){
						while(true){
							try{
								int i=client.getInputStream().read();
								if(NetworkBridge.out)System.out.print((char)i);
								server.getOutputStream().write(i);
							}catch(Exception e){
								break;
							}
						}
					}
				}.start();
				while(true){
					try{
						int i=server.getInputStream().read();
						if(NetworkBridge.out)System.out.print((char)i);
						client.getOutputStream().write(i);
					}
					catch(Exception e){
						break;
					}
				}
			}catch(Exception e){
				System.out.println("Redo");
			}
		}
	}
}
