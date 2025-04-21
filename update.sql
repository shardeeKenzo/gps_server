ALTER TABLE data ADD COLUMN timeupl integer;
CREATE INDEX timeupl_idx ON data USING btree (timeupl DESC NULLS LAST);