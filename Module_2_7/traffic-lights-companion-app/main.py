from time import sleep

import struct
import serial
import threading
import customtkinter as ctk
from dataclasses import dataclass

class AppWindow(ctk.CTk):
	commands = [
		("Run", 1),
		("Stop", 2),
		("Flashing Yellow", 3),
		("Configure", 4),
	]
	config_parameters = [
		("Green duration (ms)", "green_ms", 3000),
		("Green flashing duration (ms)", "green_flashing_ms", 1000),
		("Green flashing period (ms)", "green_flashing_period", 100),
		("Yellow duration (ms)", "yellow_ms", 2000),
		("Red duration (ms)", "red_ms", 3000),
		("Red + yellow duration (ms)", "red_and_yellow_ms", 1000),
		("Yellow flashing period (ms)", "flashing_yellow_period", 100),
	]

	buttons = {}
	config_entries = {}

	def __init__(self):
		super().__init__()

		self.title("Traffic Lights Control")
		self.geometry("600x600")
		self.grid_columnconfigure((0, 1), weight=1)

		for command_text, command_code in self.commands:
			self.buttons[command_text] = ctk.CTkButton(self, text=command_text, command=lambda code=command_code: self.button_callback(code))
			self.buttons[command_text].grid(row=command_code, column=0, padx=20, pady=5, sticky="ew")


		for idx, param in enumerate(self.config_parameters):
			frame = ctk.CTkFrame(self)
			frame.grid(row=idx, column=1, padx=20, pady=5, sticky="ew")

			label = ctk.CTkLabel(frame, text=param[0])
			label.pack(side="left", padx=5)

			entry = ctk.CTkEntry(frame)
			entry.insert(0, param[2])
			entry.pack(side="right", pady=5)

			self.config_entries[param[1]] = entry

		self.logTextbox = ctk.CTkTextbox(self, wrap="word")
		self.logTextbox.grid(row=9, column=0, padx=20, pady=5, sticky="ew", columnspan=2)

		self.ser = serial.Serial('/dev/ttyUSB0', 115200)
		# self.ser = {}

		thread = threading.Thread(target=self.read_serial)
		thread.start()

	def button_callback(self, command_code):
		command_to_send = [command_code]
		if command_code == 4:
			for config_parameter in self.config_parameters:
				command_to_send.append(int(self.config_entries[config_parameter[1]].get()))
		self.append_text("APP: Command to send: " + str(command_to_send)  + "\n")
		self.send_command(command_to_send)

	def append_text(self, text):
		self.logTextbox.insert("end", text)
		# Scroll to the newest text
		self.logTextbox.see("end")

	def send_command(self, full_command):
		command_b = struct.pack("<" + str(len(full_command)) + "H", *full_command)
		app.append_text("APP: Packed command: " + " ".join(f"{b:02X}" for b in command_b)  + "\n")
		self.ser.write(command_b)

	def read_serial(self):
		while True:
			data = self.ser.readline()
			self.append_text("COM: " + data.decode("ascii"))

app = AppWindow()
app.mainloop()