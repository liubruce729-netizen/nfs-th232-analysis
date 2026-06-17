# -*- coding: utf-8 -*-
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import os

class ExogamTopologyViewer:
    def __init__(self, root):
        self.root = root
        self.root.title("EXOGAM Topology Viewer by E.CLEMENT (IN2P3-GANIL). Rev 2026")

        # Couleur RAL 5018 (approximation : #3f8f96)
        self.bg_color = "#3f8f96"
        self.root.configure(bg=self.bg_color)

        # Variables
        self.cfg_files = []
        self.selected_file = tk.StringVar()
        self.comments = []
        self.topology_data = []
        self.flange_cristal_options = []
        self.selected_flange_cristal = tk.StringVar()

        # Interface
        self.setup_ui()

    def setup_ui(self):
        # Style pour les widgets ttk
        style = ttk.Style()
        style.configure("TFrame", background=self.bg_color)
        style.configure("TLabelFrame", background=self.bg_color, foreground="white")
        style.configure("TLabel", background=self.bg_color, foreground="white")
        style.configure("TCombobox", fieldbackground=self.bg_color, background="white")
        style.configure("TButton", background=self.bg_color, foreground="white")

        # Frame pour le menu déroulant des fichiers
        file_frame = ttk.LabelFrame(self.root, text="Select topology file")
        file_frame.pack(padx=10, pady=5, fill="x")

        # Menu déroulant des fichiers
        self.file_combobox = ttk.Combobox(file_frame, textvariable=self.selected_file)
        self.file_combobox.pack(padx=5, pady=5, fill="x")
        self.file_combobox.bind("<<ComboboxSelected>>", self.load_file)

        # Bouton pour rafraîchir la liste des fichiers
        refresh_button = ttk.Button(file_frame, text="Refresh", command=self.refresh_files)
        refresh_button.pack(padx=5, pady=5)

        # Frame pour les commentaires
        comments_frame = ttk.LabelFrame(self.root, text="Comments")
        comments_frame.pack(padx=10, pady=5, fill="both", expand=True)

        self.comments_text = tk.Text(comments_frame, wrap="word", height=10, bg=self.bg_color, fg="white")
        self.comments_text.pack(padx=5, pady=5, fill="both", expand=True)

        # Frame pour la topologie
        topology_frame = ttk.LabelFrame(self.root, text="Topology")
        topology_frame.pack(padx=10, pady=5, fill="both", expand=True)

        self.topology_text = tk.Text(topology_frame, wrap="word", height=15, bg=self.bg_color, fg="white")
        self.topology_text.pack(padx=5, pady=5, fill="both", expand=True)

        # Frame pour la sélection Flange - Cristal
        selection_frame = ttk.LabelFrame(self.root, text="Which ECC - Cristal to Board")
        selection_frame.pack(padx=10, pady=5, fill="x")

        # Menu déroulant pour Flange - Cristal
        self.flange_cristal_combobox = ttk.Combobox(selection_frame, textvariable=self.selected_flange_cristal)
        self.flange_cristal_combobox.pack(padx=5, pady=5, fill="x")
        self.flange_cristal_combobox.bind("<<ComboboxSelected>>", self.show_card_channel)

        # Label pour afficher la carte et le channel
        self.card_channel_label = ttk.Label(selection_frame, text="Board and Channel : ", background=self.bg_color, foreground="white")
        self.card_channel_label.pack(padx=5, pady=5)

        # Initialisation
        self.refresh_files()

    def refresh_files(self):
        self.cfg_files = [
            f for f in os.listdir(".")
            if f.startswith("Exogam2")
        ]
        self.file_combobox["values"] = self.cfg_files

    def load_file(self, event):
        filename = self.selected_file.get()
        if not filename:
            return

        self.comments = []
        self.topology_data = []
        self.flange_cristal_options = []

        try:
            with open(filename, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if line.startswith("//"):
                        self.comments.append(line)
                    elif line and not line.startswith("//") and line != "END":
                        self.topology_data.append(line.split())

            # Affichage des commentaires
            self.comments_text.delete(1.0, tk.END)
            self.comments_text.insert(tk.END, "\n".join(self.comments))

            # Tri des données par Flange croissant
            self.topology_data.sort(key=lambda x: int(x[0]))

            # Mise à jour des options Flange - Cristal
            self.flange_cristal_options = [f"ECC {data[0]} -  {['A', 'B', 'C', 'D'][int(data[1])]}" for data in self.topology_data]
            self.flange_cristal_combobox["values"] = self.flange_cristal_options

            # Affichage de la topologie
            self.topology_text.delete(1.0, tk.END)
            for data in self.topology_data:
                flange, cristal, numexo2, position = data
                cristal_name = ["A", "B", "C", "D"][int(cristal)]
                position_name = "channel 0" if position == "0" else "channel 8"
                self.topology_text.insert(tk.END, f"Flange: {flange}, Cristal: {cristal_name}, Board: {numexo2}, Channel: {position_name}\n")

        except Exception as e:
            messagebox.showerror("Erreur", f"Erreur lors de la lecture du fichier : {e}")

    def show_card_channel(self, event):
        selected = self.selected_flange_cristal.get()
        if not selected:
            return

        # Extraction de Flange et Cristal depuis la sélection
        flange = selected.split(" - ")[0].split(" ")[1]
        cristal = selected.split(" - ")[1].split(" ")[1]

        # Recherche de la carte et du channel
        for data in self.topology_data:
            if data[0] == flange and ["A", "B", "C", "D"][int(data[1])] == cristal:
                numexo2, position = data[2], data[3]
                position_name = "channel 0" if position == "0" else "channel 8"
                self.card_channel_label.config(text=f"Board: {numexo2}, Channel: {position_name}")
                break

if __name__ == "__main__":
    root = tk.Tk()
    app = ExogamTopologyViewer(root)
    root.mainloop()
