import telegram
import os

token = 'myToken'

bot = telegram.Bot(token=token)
updates = bot.get_updates()
chatIDs = set([u.message.chat_id for u in updates if hasattr(u.message,'chat_id')])

def send_text(text):
    for id in chatIDs:
        try:
            bot.send_message(chat_id=id, text=text)
        except:
            pass

def send_file(path):
    for id in chatIDs:
        try:
            bot.send_document(chat_id=id, document=open(path, "rb"))
        except:
            pass

def send_image(path):
    for id in chatIDs:
        try:
            bot.sendPhoto(chat_id=id, photo=open(path, "rb"))
        except:
            pass
