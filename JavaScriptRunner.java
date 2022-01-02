import javax.script.*;
import java.io.*;

public class JavaScriptRunner{
	public static void main(String[] args){
		ScriptEngineManager sem=new ScriptEngineManager();
		ScriptEngine se=sem.getEngineByName("javascript");

		if(args.length==0){
			System.err.println("\033[91m\033[1mUsage: java JavaScriptRunner [SCRIPT FILE NAME]\033[0m");
			System.exit(-1);
		}
		StringBuilder sb=new StringBuilder(args[0]);
		for(int i=1;i<args.length;i++){
			sb.append(' ').append(args[i]);
		}
		String fileName=sb.toString();
		File file=null;
		try {
			file=new File(fileName);
		}catch(Exception fe){
			System.err.printf("\033[91m\033[1m%s\033[0m\n",fe.toString());
			System.exit(-1);
		}
		if(!file.canRead()){
			System.err.printf("\033[91m\033[1mCan't open \033[93m%s\033[91m for reading\033[0m\n",fileName);
			System.exit(-1);
		}
		try {
			se.eval(new FileReader(file));
		} catch(Exception e){
			System.err.printf("\033[91m\033[1m%s\033[0m\n",e.toString());
			System.exit(-1);
		}
	}
}
