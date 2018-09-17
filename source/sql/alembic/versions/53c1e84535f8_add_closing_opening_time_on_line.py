"""Add closing& opening time on line

Revision ID: 53c1e84535f8
Revises: 3b17c15627cb
Create Date: 2015-02-10 14:31:58.121392

"""

# revision identifiers, used by Alembic.
revision = '53c1e84535f8'
down_revision = '3b17c15627cb'

from alembic import op
import sqlalchemy as sa
import geoalchemy2 as ga
from sqlalchemy.dialects import postgresql


def upgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.add_column('line', sa.Column('closing_time', postgresql.TIME(), nullable=True), schema='navitia')
    op.add_column('line', sa.Column('opening_time', postgresql.TIME(), nullable=True), schema='navitia')
    ### end Alembic commands ###


def downgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.drop_column('line', 'opening_time', schema='navitia')
    op.drop_column('line', 'closing_time', schema='navitia')
    ### end Alembic commands ###
