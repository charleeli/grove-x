# coding=utf-8
from django.db import models


# 商品标签
class WareLabel(models.Model):
    scope_type_choices = [(0, '平台内的商品'), (1, '店铺内的商品'), ]
    sub_type_choices = [(0, '白名单商品'), (1, '黑名单商品'),(2, '全部商品'), ]

    name = models.CharField("商品标签名称", max_length=200)
    scope_type = models.PositiveSmallIntegerField("使用范围", default=0, choices=scope_type_choices)
    sub_type = models.PositiveSmallIntegerField("子类型", default=0, choices=sub_type_choices)
    create_time = models.DateTimeField('创建时间', auto_now_add=True)
    update_time = models.DateTimeField('更新时间', auto_now=True)
    create_man = models.CharField("创建人", max_length=200)

    def __unicode__(self):
        return self.name


# 商品标签-商品表
class WareLabelWares(models.Model):
    ware_label_id = models.PositiveIntegerField("商品标签ID", db_index=True)
    ware_id = models.BigIntegerField("商品ID")
    ware_slug = models.CharField("商品slug", max_length=200)

    def __unicode__(self):
        return str(self.ware_label_id)+'-'+str(self.ware_id)


# 优惠券组
class CouponGroup(models.Model):
    favor_type_choices = [(0, '满减券'), ]
    scope_type_choices = [(0, '平台券'), (1, '店铺券')]
    sub_type_choices = [(0, '白名单商品'), (1, '黑名单商品'), (2, '全部商品'), ]
    scene_type_choices = [(0, '领取型'), (1, '口令型'), (2, '自动发放型'), (3, '批量导出型'), (4, '批量发放型'), ]
    verify_status_choices = [(0, '新建'), (1, '待审批'), (2, '已经上线'), (3, '不通过')]

    slug = models.CharField("slug", max_length=32)
    name = models.CharField("优惠券组名称", max_length=200)
    title = models.CharField("优惠券组标题", max_length=200)
    comment = models.CharField("优惠券组副标题", max_length=200)
    ware_label_id = models.PositiveIntegerField("商品标签ID")
    favor_type = models.PositiveSmallIntegerField("优惠类型", default=0,choices=favor_type_choices)
    scope_type = models.PositiveSmallIntegerField('使用范围', default=0, choices=scope_type_choices)
    sub_type = models.PositiveSmallIntegerField("子类型", default=0, choices=sub_type_choices)
    scene_type = models.PositiveSmallIntegerField('场景类型', default=0, choices=scene_type_choices)
    full = models.DecimalField('购满金额', max_digits=11, decimal_places=2, default=0)
    favor = models.DecimalField('免减金额', max_digits=11, decimal_places=2, default=0)
    rate = models.FloatField('打折折扣率', default=0)
    argot = models.CharField("口令", max_length=200, blank=True)
    max_count = models.PositiveIntegerField('计划最大放出数量', default=0)
    delta = models.PositiveIntegerField('分批生成时的增量', default=0)
    drawn_count = models.PositiveIntegerField('已领取的数量', default=0)
    payed_count = models.PositiveIntegerField('已支付的数量', default=0)
    create_time = models.DateTimeField('创建时间', auto_now_add=True)
    can_draw_count = models.PositiveIntegerField('每人允许领取数量', default=1)
    start_draw_time = models.DateTimeField('领用有效期开始时间', default=None, null=True, blank=True)
    end_draw_time = models.DateTimeField('领用有效期结束时间', default=None, null=True, blank=True)
    is_duration_type = models.BooleanField('使用有效期指定时长', default=False)
    duration_value = models.PositiveIntegerField('有效期时长(天)', default=0)
    start_use_time = models.DateTimeField('使用有效期开始时间', default=None, null=True, blank=True)
    end_use_time = models.DateTimeField('使用有效期结束时间', default=None, null=True, blank=True)
    verify_status = models.PositiveSmallIntegerField('审批状态', default=0, choices=verify_status_choices)
    delta_verify_status = models.PositiveSmallIntegerField('增量审批状态', default=0, choices=verify_status_choices)
    applicant = models.CharField("申请人", max_length=200)
    approver = models.CharField("审批人", max_length=200)
    modifier = models.CharField("修改人", max_length=200)
    seller_id = models.BigIntegerField('卖家id', default=0)
    url = models.URLField('活动链接', max_length=250, help_text='活动链接', default=None, null=True, blank=True)
    img = models.URLField('图片链接', max_length=250, help_text='图片链接', default=None, null=True, blank=True)
    img_width = models.PositiveIntegerField("图片宽", default=0, blank=True, editable=False)
    img_height = models.PositiveIntegerField("图片高", default=0, blank=True, editable=False)
    button_text = models.CharField("按钮文字", max_length=200)
    button_jump = models.CharField("按钮跳转", max_length=200, blank=True)
    jump_label = models.CharField('跳转类型', max_length=50, blank=True)
    jump_data = models.CharField('跳转数据', max_length=200, blank=True)
    argot_jump_label = models.CharField('跳转类型', max_length=50, blank=True)
    argot_jump_data = models.CharField('跳转数据', max_length=200, blank=True)
    update_time = models.DateTimeField('更新时间', default=None, null=True, blank=True)
    version = models.PositiveIntegerField('版本号', default=0)
    description = models.CharField("优惠券组适用范围描述", max_length=200, blank=True)

    def __unicode__(self):
        return self.name


# 优惠券
class Coupon(models.Model):
    coupon_group_id = models.PositiveIntegerField('优惠券组ID', default=0)
    code = models.CharField("优惠券码", max_length=64, db_index=True, unique=True)
    user_id = models.BigIntegerField('用户ID', default=0)
    order_id = models.CharField("订单编号", max_length=200,blank=True)
    create_time = models.DateTimeField('创建时间', auto_now_add=True)
    order_create_time = models.DateTimeField('订单生成时间', default=None, null=True, blank=True)
    drawn_time = models.DateTimeField('被领取时间', default=None, null=True, blank=True)
    frozen_time = models.DateTimeField('被冻结时间', default=None, null=True, blank=True)
    payed_time = models.DateTimeField('被支付时间', default=None, null=True, blank=True)

    def __unicode__(self):
        return self.code


# 用户-优惠券
class UserCoupons(models.Model):
    use_status_choices = [(0, '已领取'), (1, '已冻结'), (2, '已支付'),(3, '已过期'), ]

    user_id = models.BigIntegerField('用户ID', db_index=True)
    coupon_group_id = models.PositiveIntegerField('优惠券组ID', default=0)
    coupon_id = models.PositiveIntegerField('优惠券ID', default=0)
    code = models.CharField("优惠券码", max_length=200)
    create_time = models.DateTimeField('创建时间', default=None, null=True, blank=True)
    update_time = models.DateTimeField('更新时间', default=None, null=True, blank=True)
    use_status = models.PositiveSmallIntegerField("使用状态", default=0, choices=use_status_choices)
    client_id = models.CharField("客户端ID", max_length=100, blank=True, db_index=True)
    seller_id = models.BigIntegerField('卖家id', default=0)
    channel_id = models.PositiveIntegerField('渠道id', default=0)

    def __unicode__(self):
        return str(self.user_id)+'-'+self.code
