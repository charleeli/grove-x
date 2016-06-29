from django.contrib import admin

# Register your models here.
from .models import WareLabel,WareLabelWares,CouponGroup,Coupon,UserCoupons

class WareLabelAdmin(admin.ModelAdmin):
    list_display = [
        'id','name',
        'scope_type','sub_type',
        'create_time','update_time',
    ]

    list_filter = [
        'scope_type','sub_type',
    ]

    search_fields = ['name',]


class WareLabelWaresAdmin(admin.ModelAdmin):
    list_display = [
        'ware_label_id',
        'ware_id','ware_slug',
    ]

    list_filter = [
        'ware_label_id',
    ]

    search_fields = ['ware_label_id',]


class CouponAdmin(admin.ModelAdmin):
    list_display = [
        'id','code','coupon_group_id','user_id','order_id',
        'create_time','order_create_time',
        'drawn_time','frozen_time','payed_time',
    ]

    list_filter = [
        'coupon_group_id',
    ]

    search_fields = ['code']


class CouponGroupAdmin(admin.ModelAdmin):
    list_display = [
        'id','name','title','comment','ware_label_id',
        'favor_type','scope_type','sub_type','scene_type',
        'full','favor','rate','argot',
        'max_count','delta','drawn_count','payed_count',
        'create_time','start_draw_time','end_draw_time','can_draw_count',
        'is_duration_type','duration_value','start_use_time','end_use_time',
        'verify_status','applicant','approver','modifier',
        'seller_id','url','img',
    ]

    list_filter = [
        'favor_type','scope_type','sub_type','scene_type',
    ]

    search_fields = ['name']
    ordering = ['-create_time']
    list_per_page = 20


class UserCouponsAdmin(admin.ModelAdmin):
    list_display = [
        'user_id','coupon_group_id','coupon_id','code',
        'create_time','update_time','use_status',
    ]

    list_filter = [
        'use_status',
    ]

    search_fields = ['user_id']


admin.site.register(Coupon,CouponAdmin)
admin.site.register(WareLabel,WareLabelAdmin)
admin.site.register(WareLabelWares,WareLabelWaresAdmin)
admin.site.register(CouponGroup,CouponGroupAdmin)
admin.site.register(UserCoupons,UserCouponsAdmin)