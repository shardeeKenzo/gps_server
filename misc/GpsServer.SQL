DROP INDEX IF EXISTS time_index;
DROP INDEX IF EXISTS sensID_index;

DROP TABLE IF EXISTS accounts;
DROP TABLE IF EXISTS sensors;
DROP TABLE IF EXISTS points;

CREATE TABLE accounts(
      ID        SERIAL
    , login     varchar(50)  NOT NULL
    , password  varchar(100) NOT NULL

    , PRIMARY KEY (ID)
);

CREATE TABLE sensors(
      ID        SERIAL       NOT NULL
    , imei      varchar(100) NOT NULL
    , accID     INTEGER      NOT NULL
    
    , PRIMARY KEY (ID)
    , FOREIGN KEY (accID) REFERENCES accounts (ID)
);

-- Table: points

CREATE TABLE data(
      ID        SERIAL
    , sensID    integer NOT NULL
    
    , lat       integer NOT NULL
    , lon       integer NOT NULL 
    , alt       integer NOT NULL
        
    , time      integer NOT NULL
    
    , speed     integer NOT NULL
    , direction integer NOT NULL
    , satcnt    integer

    , signal    BOOL    NOT NULL

    , PRIMARY KEY (ID)
    , FOREIGN KEY (sensID) REFERENCES sensors(ID)
);

CREATE INDEX time_index  ON data USING btree(time);
CREATE INDEX sensID_index ON data USING btree(sensID);
