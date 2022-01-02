public class Time{
	public static void main(String[] args){
		int h=0,min=0,sec=0;
		StringBuilder sb=new StringBuilder();
		System.out.println("开始计时...");
		while(true){
			sb.setLength(0);
			sb.append("\033[100D").append(h).append(':').append(min<10?'0':"").append(min).append(':').append(sec<10?'0':"").append(sec);
			System.out.print(sb);
			try{
				sec++;
				if(sec==60){
					sec=0;
					min++;
					if(min==60){
						min=0;
						h++;
					}
				}
				Thread.sleep(1000);
			}catch(Exception e){}
		}
	}
}
