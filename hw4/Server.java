import java.net.ServerSocket;
import java.net.Socket;
import java.io.PrintWriter;
import java.util.Scanner;
import java.util.Arrays;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
public class Server {
    public static void main(String[] args) throws Exception {
        try (ServerSocket listen = new ServerSocket(1234)) {
            System.out.println("Tic Tac Toe Server is running...");
            ExecutorService pool = Executors.newFixedThreadPool(10); //建立執行緒pool
            while (true) {
                Game game = new Game();
                //使用2個執行緒處理兩個client
                pool.execute(game.new Player(listen.accept(), 'X')); //首先連線進來的player是 'X'
                pool.execute(game.new Player(listen.accept(), 'O')); //後面進來的player是 'O'
            }
        }
    }
}
class Game{
    private Player[] board = new Player[9];
    private Player current_player;
    private Player opponent_player;
    public synchronized void move(int position,Player p){
        if (p != current_player) { //如果這局不是輪到你(other's turn)
            throw new IllegalStateException("Not your turn");
        } else if (opponent_player == null) { //如果對方還沒上線
            throw new IllegalStateException("You don't have an opponent yet");
        } else if(board[position] != null){  //這一格已經下過了
            throw  new IllegalStateException("Cell already occupied");
        }
        board[position] = p;
        p.position = position;
    }
    public boolean isWin(){ //判斷是否勝利
        return ((board[0]!=null && board[0]==board[1] && board[0]==board[2]) || (board[3]!=null && board[3]==board[4] && board[3]==board[5]) 
            || (board[6]!=null && board[6]==board[7] && board[6]==board[8]) || (board[0]!=null && board[0]==board[3] && board[0]==board[6])
            || (board[1]!=null && board[1]==board[4] && board[1]==board[7]) || (board[2]!=null && board[2]==board[5] && board[2]==board[8]) 
            || (board[0]!=null && board[0]==board[4] && board[0]==board[8]) || (board[2]!=null && board[2]==board[4] && board[2]==board[6]));
    }
    class Player implements Runnable{
        Socket socket;
        char mark;
        Scanner recv_in;
        PrintWriter send_out;
        int position;
        public Player(Socket s,char m){
            this.socket = s;
            this.mark = m; //記號
        }
        @Override
        public void run(){
            try{
                recv_in = new Scanner(socket.getInputStream());
                send_out = new PrintWriter(socket.getOutputStream(),true);
            }catch(Exception e){
                e.printStackTrace();
            }
            connect_setting(); //設定先連線進來的先下(current player)
            send_out.println("Welcome "+this.mark);
            if(this.mark == 'O'){
                current_player.send_out.println("opponent O is already online.");
                send_out.println("opponent X is already online.");
            }
            while(recv_in.hasNextLine()){
                String response = recv_in.nextLine();
                if(response.startsWith("Falls on ")){ //接收到下的位置
                    char c = response.charAt(9);
                    int num = Character.getNumericValue(c);
                    try{
                        move(num, this);
                        this.send_out.println("Valid position"); 
                        opponent_player.send_out.println("Opponent move "+this.position); //告訴對方我方下的位置
                        swap_turn(); //換對方下
                    }catch(IllegalStateException e){ //如果move()出現異常
                        send_out.println("Message:"+e.getMessage()); //送出異常訊息
                    }
                }
                if(isWin()){ //判斷是否有人勝出
                    send_out.println("Victory");
                    current_player.send_out.println("Defeat");
                }
                else if(Arrays.stream(board).allMatch(p -> p != null)){ //平手的情況
                    send_out.println("Draw");
                    current_player.send_out.println("Draw"); 
                }
                if(response.equals("Quit")){
                    try{
                        socket.close(); //離線
                    }catch(Exception e){
                        e.printStackTrace();
                    }
                    break;
                }
            }
        }
        private synchronized void swap_turn(){
            Player temp = current_player;
            current_player = opponent_player;
            opponent_player = temp;
        }
        private synchronized void connect_setting(){
            if(this.mark == 'X') {
                current_player = this;
            }
            else if(this.mark == 'O'){ 
                opponent_player = this;
            }
        }
    }
}