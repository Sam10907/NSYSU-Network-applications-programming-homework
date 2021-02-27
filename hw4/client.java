import java.awt.GridLayout;
import java.awt.Font;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import java.net.Socket;
import java.io.PrintWriter;
import java.util.Scanner;

public class client{
    class square extends JPanel {
        private static final long serialVersionUID = 1L;
        JLabel label = new JLabel();

        public square() {
            setBackground(Color.white); //將方塊的背景設為白色
            label.setFont(new Font("Arial", Font.BOLD, 100));
            add(label);
        }
        public void setText(char text) {
            label.setForeground(text == 'X' ? Color.RED : Color.BLUE); // 'X'為紅色 'O'為藍色
            label.setText(text + ""); //把'X' or 'O'加入label裡
        }
    }

    private square[] board = new square[9];
    private square current_click_square;
    private JFrame frame = new JFrame("Tic Tac Toe"); //整個遊戲的畫面框架（frame）
    private JLabel Mes_label = new JLabel("Wait..."); //遊戲進行中所產生的訊息
    private Socket socket;
    private Scanner recv_in;
    private PrintWriter send_out;
    public client(String address,int port) throws Exception{
        socket = new Socket(address,port);
        recv_in = new Scanner(socket.getInputStream());
        send_out = new PrintWriter(socket.getOutputStream(),true);

        Mes_label.setBackground(Color.lightGray);//遊戲訊息的背景顏色為輕灰色
        frame.getContentPane().add(Mes_label,BorderLayout.SOUTH); //將遊戲訊息框放在最底下
        JPanel board_panel = new JPanel();
        board_panel.setBackground(Color.black);
        board_panel.setLayout(new GridLayout(3, 3, 2, 2)); // 3*3的網格
        for(int i = 0;i < 9;i++){
            final int j = i;
            board[i] = new square();
            board[i].addMouseListener(new MouseAdapter() { //為每一個方塊添加滑鼠點擊事件
                public void mousePressed(MouseEvent e) {
                    send_out.println("Falls on " + j); //點擊後送出位置
                    current_click_square = board[j]; //將目前點擊的這個方塊保存起來
                }
            });
            board_panel.add(board[i]); //將設置好的每一個方塊添加到網格裡 形成3*3的遊戲盤
        }
        frame.getContentPane().add(board_panel,BorderLayout.CENTER); //將遊戲盤添加進遊戲畫面框架裡
    }
    public void playing() throws Exception{
        try{
            String response = recv_in.nextLine();
            char current_mark = response.charAt(8);
            char opponent_mark = current_mark == 'X' ? 'O' : 'X';
            frame.setTitle("Tic Tac Toe: Player " + current_mark); 
            while(true){
                response = recv_in.nextLine();
                if(response.startsWith("Valid position")){ //有效的位置
                    Mes_label.setText("Valid position,please wait opponent...");
                    current_click_square.setText(current_mark);
                    current_click_square.repaint();
                }
                else if(response.startsWith("Opponent move")){ //對方下的位置
                        int position = Integer.parseInt(response.substring(14));
                        board[position].setText(opponent_mark);
                        board[position].repaint();
                        Mes_label.setText("Your turn...");
                }
                else if(response.startsWith("Message")){ //server傳來的一些異常訊息
                    Mes_label.setText(response.substring(8));
                }
                else if(response.equals("opponent O is already online.")){
                    Mes_label.setText("opponent O is already online.");
                }
                else if(response.equals("opponent X is already online.")){
                    Mes_label.setText("opponent X is already online.");
                }
                else if (response.equals("Victory")){ //勝利
                        JOptionPane.showMessageDialog(frame,"You are winner!"); //跳出勝利訊息對話框
                        break;
                }
                else if (response.equals("Defeat")){ //敗北
                        JOptionPane.showMessageDialog(frame,"You lost.QQ"); //跳出敗北訊息對話框
                        break;
                }
                else if (response.equals("Draw")){ //平手
                        JOptionPane.showMessageDialog(frame,"Draw"); //跳出平手訊息對話框
                        break;
                }
            }
            send_out.println("Quit");
        }catch(Exception e){
            e.printStackTrace();
        }
        finally{
            socket.close(); //離線
            frame.dispose();
        }
    }
    public static void main(String[] args) throws Exception {
        Scanner scanner = new Scanner(System.in); 
        String address = scanner.nextLine(); //輸入server的ip address
		client TicTacToe_client = new client(address.toString(),1234);
        TicTacToe_client.frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        TicTacToe_client.frame.setSize(640, 640);
        TicTacToe_client.frame.setVisible(true);
        TicTacToe_client.frame.setResizable(false);
        TicTacToe_client.playing(); //開始遊玩
        scanner.close();
    }
}