from os import wait
import tkinter as tk
import socket
import math

from PIL import ImageTk
from PIL import Image

class Application(tk.Frame):
    def __init__(self, master,filename1,filename2,filename3,filename4):
        super().__init__(master)
        self.master = master
        self.filename1 = filename1
        self.filename2 = filename2
        self.filename3 = filename3
        self.filename4 = filename4
        self.canvas1 = tk.Canvas(master, width=250, height=250, bg = 'black')
        self.canvas2 = tk.Canvas(master, width=250, height=250, bg = 'black')
        self.canvas3 = tk.Canvas(master, width=250, height=250, bg = 'black')
        self.canvas4 = tk.Canvas(master, width=180, height=250, bg = 'white')

        self.canvas_data1 = tk.Canvas(master, width=250, height=50, bg = 'white')
        self.canvas_data2 = tk.Canvas(master, width=250, height=50, bg = 'white')
        self.canvas_data3 = tk.Canvas(master, width=250, height=50, bg = 'white')
        self.canvas_data4 = tk.Canvas(master, width=180, height=50, bg = 'white')

        self.label1=tk.Label(master,font='ariel 20',fg='black',text='Pitch')
        self.label2=tk.Label(master,font='ariel 20',fg='black',text='Roll')
        self.label3=tk.Label(master,font='ariel 20',fg='black',text='Yaw')
       
        self.label1.grid(row=0,column=0)
        self.label2.grid(row=0,column=1)
        self.label3.grid(row=0,column=2)

        self.create_widgets()
    
        self.update = self.draw_drone().__next__
        master.after(1000, self.update)

    def create_widgets(self):
        self.hi_there = tk.Button(self)
        self.hi_there["command"] = self.say_hi
        self.hi_there.pack(side="top")

        self.quit = tk.Button(self, text="QUIT", fg="red", command=self.master.destroy)
        self.quit.pack(side="bottom")

    def draw(self):
        image =  Image.open(self.filename1)
        angle = 0
        while True:
            tkimage = ImageTk.PhotoImage(image.rotate(angle))
            canvas_obj = self.canvas.create_image(200, 200, image=tkimage)
            self.master.after_idle(self.update)
            yield
            self.canvas.delete(canvas_obj)
            angle += 10
            angle %= 360 

    def draw_drone(self):
        image_roll = Image.open(self.filename1)
        image_pitch = Image.open(self.filename2)
        image_yaw = Image.open(self.filename3)
        image_height = Image.open(self.filename4)

        self.canvas1.grid(row=1, column=0)
        self.canvas2.grid(row=1, column=1)
        self.canvas3.grid(row=1, column=2)
        self.canvas4.grid(row=1, column=3)

        self.canvas_data1.grid(row=2, column=0)
        self.canvas_data2.grid(row=2, column=1)
        self.canvas_data3.grid(row=2, column=2)
        self.canvas_data4.grid(row=2, column=3)

        fi,psi,theta = 0, 0, 0
        wind_pitch, wind_roll = 0, 0
        flight_height,vertical_speed = 0, 0
        while True:
            tkimage_roll = ImageTk.PhotoImage(image_roll.rotate(fi)) # degree
            tkimage_pitch = ImageTk.PhotoImage(image_pitch.rotate(psi))
            tkimage_yaw = ImageTk.PhotoImage(image_yaw.rotate(theta))
            
            if vertical_speed >= 0:
                tkimage_height = ImageTk.PhotoImage(image_height.rotate(0))
            else :
                tkimage_height = ImageTk.PhotoImage(image_height.rotate(180)) 

            canvas_obj_roll = self.canvas1.create_image(125, 125, image=tkimage_roll)
            canvas_obj_pitch = self.canvas2.create_image(125, 125, image=tkimage_pitch)
            canvas_obj_yaw = self.canvas3.create_image(125, 125, image=tkimage_yaw)
            canvas_obj_hight = self.canvas4.create_image(90, 125, image=tkimage_height)

            canvas_data_roll = self.canvas_data1.create_text(125,20,text="current value: " + str(fi) + "\nwind value: " + str(wind_roll))
            canvas_data_pitch = self.canvas_data2.create_text(125,20,text="current value: " + str(psi)+ "\nwind value: " + str(wind_pitch))
            canvas_data_yaw = self.canvas_data3.create_text(125,20,text="current value: " + str(theta))
            canvas_obj_height = self.canvas_data4.create_text(90,20,text="flight height: " + str(flight_height)+ "\nvertical speed: " + str(vertical_speed))
            
            current_angles = self.receive_data()
            self.master.after_idle(self.update)
            yield
            self.canvas1.delete(canvas_obj_roll)
            self.canvas2.delete(canvas_obj_pitch)
            self.canvas3.delete(canvas_obj_yaw)
            self.canvas4.delete(canvas_obj_hight)

            self.canvas_data1.delete(canvas_data_roll)
            self.canvas_data2.delete(canvas_data_pitch)
            self.canvas_data3.delete(canvas_data_yaw)
            self.canvas_data4.delete(canvas_obj_height)

            fi = int(current_angles[0]/100) # degrees
            psi = int(current_angles[1]/100) # degrees
            theta = int(current_angles[2]/100) # degrees

            wind_pitch = int(current_angles[3]/100)
            wind_roll = int(current_angles[4]/100)
            flight_height = current_angles[5]
            vertical_speed = current_angles[6]

    def say_hi(self):
        print("hi there, everyone!")
    
    def receive_data(self):
        bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
        message = bytesAddressPair[0]
        address = bytesAddressPair[1]
        clientMsg = "Message from Client:{}".format(message)
        clientIP  = "Client IP Address:{}".format(address)

        pitch = int.from_bytes( message[0:2], 'little', signed=True )
        roll = int.from_bytes( message[2:4], 'little', signed=True )
        yaw = int.from_bytes( message[4:6], 'little', signed=True )

        pitch_noise = int.from_bytes( message[6:8], 'little', signed=True )
        roll_noise = int.from_bytes( message[8:10], 'little', signed=True )
        height = int.from_bytes( message[10:12], 'little', signed=True )
        vertical_velocity = int.from_bytes( message[12:14], 'little', signed=True )

        UDPServerSocket.sendto(bytesToSend, address)
        return (pitch, roll, yaw,pitch_noise,roll_noise, height, vertical_velocity)

localIP     = "127.0.0.1"
localPort   = 1102
bufferSize  = 14
msgFromServer = "Hello UDP Client"
bytesToSend   = str.encode(msgFromServer)

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening")
# Listen for incoming datagrams


def main():
    root = tk.Tk()
    root.title("Drone simulator - visualization")
    #root.geometry('780x300')
    app = Application(root,'../drone3.png','../drone3.png','../drone4.png','../arrow.png')
    #app.create_lines()
    root.mainloop()

if __name__ == '__main__':
    main()