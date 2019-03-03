// steal.c

#include <ansi.h>
#include <login.h>

inherit F_CLEAN_UP;

void create() {seteuid(getuid());}

int main(object me, string arg)
{
    string what, who;
    object ob, victim;
    int sp, dp, skill, forcesk;
    string stealer_class, stealer_race;

    stealer_class = me->query_class();
    stealer_race = me->query_race();
    forcesk = (me->query_skill("force", 1)/25) + 4;

    if( me->is_busy() ) return notify_fail("�A�{�b�S���šT\n");

    if( me->query("life_form") == "ghost" )
	return notify_fail("�H���F�N�@�F�ʤF, �֥h�䫰���_���a!!\n");

    if( environment(me)->query("no_fight") 
    || environment(me)->query("no_steal") )
	return notify_fail("�o�̸T����ѡC\n");

    if( me->skill_mapped("stealing")==0 )
	return notify_fail("�A�����ϥ� (enable) ����ŪŤ���!\n");
                
    if( me->query_temp("stealing") )
	return notify_fail("�A�w�g�b����|�U��F�T\n");

    if( !arg || sscanf(arg, "%s from %s", what, who)!=2 ) 
	return notify_fail("���O�榡�Rsteal <���~> from <�H��>\n");

    victim = present(who, environment(me));
    if( !victim || victim==me || !victim->is_character() )
	return notify_fail("�A�Q���Ѫ���H���b�o�̡C\n");
    if( !wizardp(me) && wizardp(victim) )
	return notify_fail("���a���ఽ�Ův���W���F��C\n");

    // add by dragoon
    if( userp(victim) && !interactive(victim) ) 
	return notify_fail("�o�H�{�b���b�u�W�C\n");

    // Don't let NPC steal NPC.
    if( !userp(me) && !userp(victim) )
	return 0;

/*
        if( userp(victim))
           { if(victim->query("level") < 5)
                return notify_fail("�L���ŤӧC, ���n���L�a�C\n"); }

        if( me->query("level") < 5)
           { if(userp(victim))
                return notify_fail("�A���ŤӧC, ���j�I�A�X�Ӱ��ѧa�C\n"); }     
*/
    if( !ob = present(what, victim) ) {
	object *inv;
        inv = all_inventory(victim);
        if( !sizeof(inv) )
            return notify_fail( victim->name() + "���W�ݰ_�ӨS������ȿ����F��n���C\n");
                ob = inv[random(sizeof(inv))];
    }

    skill = me->query_skill("stealing");
    // change while to if -Dragoon
    if( me->query_stat("gin") < skill/3 )
        skill /= 2;
    me->consume_stat("gin", skill/3);

    // reset the power of steal -dragoon
    // steal�ޯ�C���ɭ�, ���������өM�w�O�����v�Tsteal����O, ��steal
    // �ޯ��F�۷��{�׫�, �ޯ��steal��O���v�T�}�l�����ݩ�
    sp = skill*5 + (int)me->query("dex")*10 + (int)me->query("cps")*5;

    // ¾�~���[��O�����@
    if( me->query("class") == "thief" ) 
	sp += skill*5 + (int)me->query("dex")*10;

    // �ݩʩM�ޯ�~�����T�w����O���{, �����P�p���b�ޯ���ݩʽm�쳻�I��
    // , ��steal��O���t���h�Ѻ몺���C�Ӧ��Үt��, ��ƭȪ����C��steal��
    // �O�����P�h�Ū��v�T, ��V��, ��O�e�G���żƤW�@
    sp += 1 + (int)me->qeury_stat("gin")*(int)me->qeury_stat("gin")/600;
	    
    if( sp < 1 ) sp = 1;
    // jo�@�ڽ�P�ѥͪ���O, ��O�Pforce����, �����P�򥻪���O����4, �Y
    // force���S�m�����p�U, jo��steal��O�٬O��O�H���X (skill+dex)*4
    // �̰����Ƭ�: 150/25+4=10, �Hsteal skill�W��200, dex���]30, ����steal
    // ��O��@��ر�sp���X2300
    if( stealer_race == "jiaojao" ) 
	sp += (skill + (int)me->query("dex")) * forcesk;
    if( me->is_fighting() && stealer_race != "jiaojao" ) {
	sp /= 2;
        me->start_busy(3);
    }
    if( me->is_fighting() && stealer_race == "jiaojao" ) {
        me->start_busy(1);
    }

    dp = (int)victim->query_stat("sen") * 3 + (int)ob->weight() / 20;
    dp += (int)victim->qeury_stat("gin")*(int)victim->qeury_stat("gin")/700;
    
    if( stealer_race == "jiaojao" ) dp += (int)victim->query_stat("gin");
    if( victim->is_fighting() ) dp *= 2;
    if( ob->query("equipped") ) dp *= 3;
    // max dp for rr += 1500
    if ( victim->query("race") == "rainnar" ) {
	if ( present("black viper", victim) ) dp += 100;
        if ( present("red viper", victim) ) dp += 300;
        if ( present("green viper", victim) ) dp += 300;
        if ( present("white viper", victim) ) dp += 400;
        if ( present("yellow viper", victim) ) dp += 400;
    }

    write("�A�����n��a�C�C�a��" + victim->name() + "�M���ݾ��|�U�� ...\n\n");

    me->set_temp("stealing", 1);
    me->delete_temp("pending/hidden");
    call_out( "compelete_steal", 3, me, victim, ob, sp, dp);

    return 1;
}

private void compelete_steal(object me, object victim, object ob, int sp, int dp)
{
    if( !me ) return;

    me->delete_temp("stealing");

    if( !victim || environment(victim) != environment(me) ) {
	tell_object(me, "�ӥi���F�M�A�U�⪺�ؼФw�g���F�C\n");
        return;
    }

    if( victim->detect_steal(me, ob, sp) ) return;

    if( objectp(ob) && (!living(victim) || (random(sp+dp) > dp) )) {
	if ( ob->query("id") == "black viper"
        || ob->query("id") == "green viper"
        || ob->query("id") == "white viper"
        || ob->query("id") == "red viper"
        || ob->query("id") == "yellow viper" ) {
            tell_object(me, "�A�N��@��" + ob->name() +
            "�M�e�i�ۤ@�f�r���V�A�r�ӡM�~���A�s�h�n�X�B�C\n");
            return;
        }
        if( ob->query("no_steal") ) {
            tell_object(me, "�o�F�褣�ఽ�C\n");
            return;
        }
        if( !ob->move(me) ) {
            tell_object(me, "�A�N��@" + ob->query("unit") + ob->name()
                + "�M�i�O��A�Ө��ӭ��F�M���o�����C\n");
            return;
        }
        tell_object(me, HIW "�o��F�T\n\n" NOR);
        tell_object(me, "�A���\\�a����@" + ob->query("unit") + ob->name() + "�T\n");
        if( living(victim) ) {
            int gain;
            me->improve_skill("stealing", me->query_attr("int"));
            gain = ob->query("value") / (me->query_skill("stealing")+50);
            if( gain > 100 ) gain = 100;
            me->gain_score("thievery", gain);
            if( userp(victim) )
                me->gain_score("mortal sin", random(me->query_skill("stealing") / 2) + 1 );
            }
            if( random(sp) < dp/2 )
                message("vision", "�A�ݨ�" + me->name() + "���������a�q"
                    + victim->name() + "���W�����F�@" + ob->query("unit")
                    + ob->name() + "�T\n", environment(me), ({ me, victim }) );
#ifdef SAVE_USER
    victim->save();
#endif
	}
        else {
            if( !ob || random(sp) > dp/2 ) {
                if ( present("black viper", victim)
                || present("red viper", victim)
                || present("green viper", victim)
                || present("white viper", victim)
                || present("yellow viper", victim) ) {
                    tell_object(me, "��T�a�@�n�M" + victim->name() + 
			"���W��M«�X�@�������R�H���D�M�������ۧA�T\n"
			"�A�~�o�����Y��C\n");
                    return; 
		}
                else {
	            tell_object(me, victim->name() + "���g�N�a�@���Y�M�A�榣�N���Y�F�^�h�T\n"
                         "�٦n�M�S���Q�o�{�C\n");
                    return; 
		}
            }
            tell_object(me, HIR "�V�|�T�A����F�T\n\n" NOR);
            if ( present("black viper", victim)
            || present("red viper", victim)
            || present("green viper", victim)
            || present("white viper", victim)
            || present("yellow viper", victim) ) {
                message_vision("$N���W���D�r�M�r�V$n�M��$N��ı$n���⥿���$N��"
                + ob->name() + "�T\n\n"
                +  "$N�ܹD�R�u�F����T�v\n\n", victim, me); 
	    }
            else {
                message_vision("$N�@�^�Y�M���n�o�{$n���⥿���$P���W��" + ob->name() + "�T\n\n"
                        + "$N�ܹD�R�u�F����T�v\n\n", victim, me);
            }
            me->improve_skill("stealing", random(me->query_attr("int")));
            me->gain_score("thievery", 1);
            if( userp(victim) ) victim->fight_ob(me);
        else {
                victim->accept_kill(me);
                victim->kill_ob(me);
        }
        me->fight_ob(victim);
        if( me->query_race() != "jiaojao" ) me->start_busy(3);
        if( me->query_race() == "jiaojao" ) me->start_busy(1);
    }
}

int help(object me)
{
write(@HELP
���O�榡 : steal <�Y��> from <�Y�H>

�o�ӫ��O���A�����|����L�H���W���F��C���\�F, ���M�A�N����o
�Ӽ˪��~�C�i�O, ��������, �H�`���������ɭ�, ���A���Ѯɷ��M�N
�o�I�X�N��, �ܩ�O����N��......�a�A�ۤv�h�o���o�C

���A�O�s��o��¾�~�ɡA�۹���L��¾�~�|�����������s��O�A�]
����e�����\�A�Ʀܥi�H�N�ؼЪ��K���窫�q�q�����A�P�˪��A���
�O�H�Q�ʧA���W�F�誺�����ɡA�]����e����ı�A���s��Q�n���s��
���ɭԡA�o�ɴN��ݽ֪��\�O���j�F�C

HELP
    );
    return 1;
}