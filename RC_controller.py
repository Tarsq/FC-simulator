import tkinter as tk
import socket
from tkinter.constants import HORIZONTAL


serverAddressPort = ("127.0.0.1", 1100)
bufferSize = 12
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

class Application(tk.Frame):
    def __init__(self,master):
        super().__init__(master)
        self.master = master
        self.create_gimbals()

        master.after(100, self.update)

    def create_gimbals(self):
        self.throttle_slider = tk.Scale(self.master, from_= 18000, to=-18000, command=self.send_command )
        self.roll_slider = tk.Scale(self.master, from_= -18000, to=18000, orient=HORIZONTAL, command=self.send_command )
        self.pitch_slider = tk.Scale(self.master, from_= 18000, to=-18000 , command=self.send_command )
        self.yaw_slider = tk.Scale(self.master, from_= -18000, to=18000, orient=HORIZONTAL , command=self.send_command )
        
        self.throttle_slider.grid(row=0,column=0)
        self.yaw_slider.grid(row=1,column=0)

        self.pitch_slider.grid(row=0,column=2)
        self.roll_slider.grid(row=1,column=2)
        
    def send_command(self,val):
        #self.label=tk.Label(self.master,font='ariel 20',fg='black',text="yaw"+ str(self.throttle_slider.get()))
        #self.label.grid(row=3,column=0)
        #bytesToSend = int.to_bytes(self.throttle_slider.get())

        #tab_bytes= [self.pitch_slider.get(), self.roll_slider.get(), self.yaw_slider.get()]

        pitch_byte = self.pitch_slider.get().to_bytes(2, 'little' , signed = True )
        roll_byte = self.roll_slider.get().to_bytes(2, 'little' , signed = True )
        yaw_byte = self.yaw_slider.get().to_bytes(2, 'little' , signed = True )
        throttle_byte = self.throttle_slider.get().to_bytes(2, 'little' , signed = True )


        bytesToSend = pitch_byte + roll_byte + yaw_byte + throttle_byte

        # print(bytesToSend)
        UDPClientSocket.sendto(bytesToSend, serverAddressPort)
        msgFromServer = UDPClientSocket.recvfrom(bufferSize)
        msg = ("Message from Server :", int.from_bytes(msgFromServer[0], "little"))
        

def main():
    root = tk.Tk()
    root.title('RC_controller')
    app = Application(root)
    root.mainloop()

if __name__== '__main__':
    main()
    