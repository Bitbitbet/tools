import java.net.*;
import java.util.*;
import java.io.*;

public class Server{
	ServerSocket ss;
	ArrayList<Thread> clientThreads;
	public static int id=0;
	public static Socket clientTmp;
	public static void main(String[] args) {
		new Server().init(args);
	}
	public void init(String[] args){
		if(args.length!=1){
			System.err.println("Usage: java [port]");
			System.exit(1);
		}
		try{
			ss = new ServerSocket(Integer.parseInt(args[0]));
			ss.setSoTimeout(0);
		}catch(NumberFormatException nfe){
			System.err.println("Port must be an integer");
			System.exit(2);
		}catch(IOException ioe){
			ioe.printStackTrace();
			System.exit(3);
		}

		clientThreads=new ArrayList<Thread>();
		while(true){
			try{
				clientTmp=ss.accept();
			}catch(IOException ioe){
				ioe.printStackTrace();
				System.exit(4);
			}
			Thread clientThread=new Thread(new Runnable(){
				public int ID;
				public Socket client;
				public InputStream is;
				public InputStreamReader isr;
				public void run(){
					client=clientTmp;
					ID=id;
					id++;
					StringBuilder sb=new StringBuilder("Client ");
					sb.append(ID);
					sb.append(" from \033[94m");
					sb.append(client.getInetAddress().toString());
					sb.append("\033[0m connected");
					log(ID, sb.toString());
					try{
						is=client.getInputStream();
						isr=new InputStreamReader(is);
					}catch(IOException e){e.printStackTrace();System.exit(5);}
					sb.setLength(0);
					while(true){
						char data=0;
						try{
							data=(char)isr.read();
							if(data==(char)-1)break;
						}catch(IOException e){
							e.printStackTrace();
							break;
						}
						if(data=='\n'){
							log(ID,sb.toString());
							sb.setLength(0);
						}else {
							sb.append(data);
						}
					}
				}
			});
			clientThread.start();
			clientThreads.add(clientThread);
		}
	}

	public static void log(int ID,String info){
		System.out.printf("[Thread %d] %s\n",ID,info);
	}
}
