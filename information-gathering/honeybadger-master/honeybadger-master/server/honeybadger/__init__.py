from flask import Flask
from flask_bcrypt import Bcrypt
from flask_sqlalchemy import SQLAlchemy
from honeybadger.utils import Logger
import os

basedir = '/tmp'#os.path.abspath(os.path.dirname(__file__))

# configuration
SQLALCHEMY_DATABASE_URI = 'sqlite:///' + os.path.join(basedir, 'data.db')
DEBUG = True
SECRET_KEY = 'development key'
SQLALCHEMY_TRACK_MODIFICATIONS = False

app = Flask(__name__)
app.config.from_object(__name__)
bcrypt = Bcrypt(app)
db = SQLAlchemy(app)
log = Logger()

import models
import views

def initdb(username, password):
    db.create_all()
    import binascii
    u = models.User(email=username, password_hash=bcrypt.generate_password_hash(binascii.hexlify(password)), role=0, status=1)
    db.session.add(u)
    db.session.commit()
    print 'Database initialized.'
    # remove below for production
    t = models.Target(name='demo', guid='aedc4c63-8d13-4a22-81c5-d52d32293867')
    db.session.add(t)
    db.session.commit()
    b = models.Beacon(target_guid='aedc4c63-8d13-4a22-81c5-d52d32293867', agent='HTML', ip='1.2.3.4', port='80', useragent='Mac OS X', comment='this is a comment.', lat='38.2531419', lng='-85.7564855', acc='5')
    db.session.add(b)
    db.session.commit()
    b = models.Beacon(target_guid='aedc4c63-8d13-4a22-81c5-d52d32293867', agent='HTML', ip='5.6.7.8', port='80', useragent='Mac OS X', comment='this is a comment.', lat='34.855117', lng='-82.114192', acc='1')
    db.session.add(b)
    db.session.commit()

def dropdb():
    db.drop_all()
    print 'Database dropped.'
